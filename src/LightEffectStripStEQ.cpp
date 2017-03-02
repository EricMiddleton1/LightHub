#include "LightEffectStripStEQ.hpp"

#include <cmath>
#include <iostream>
#include <algorithm>

LightEffectStripStEQ::LightEffectStripStEQ(
	std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer, size_t _maxBins)
	:	ILightEffect({LightNode::Type::DIGITAL})
	,	spectrumAnalyzer(_spectrumAnalyzer)
	,	maxBins{_maxBins}
	,	leftAvgEnergy{0.}
	,	rightAvgEnergy{0.} {
	
}

LightEffectStripStEQ::~LightEffectStripStEQ() {

}

void LightEffectStripStEQ::addNode(const std::shared_ptr<LightNode>& node) {
	ILightEffect::addNode(node);

	size_t binCount = std::min((size_t)node->getWidth(), maxBins);

	leftValues.insert({node, std::vector<double>(binCount/2)});
	rightValues.insert({node, std::vector<double>(binCount/2)});
}

void LightEffectStripStEQ::removeNode(std::shared_ptr<LightNode> node) {
	ILightEffect::removeNode(node);

	leftValues.erase(node);
	rightValues.erase(node);
}

void LightEffectStripStEQ::update() {
	auto leftSpec = spectrumAnalyzer->getLeftSpectrum();
	auto rightSpec = spectrumAnalyzer->getRightSpectrum();

	for(auto& node : nodes) {
		auto& leftHeights = leftValues.at(node);
		auto& rightHeights = rightValues.at(node);

		size_t binCount = leftHeights.size();
		
		auto& strip = node->getLightStrip();
		strip.setAll({});

		for(int i = 0; i < binCount; ++i) {
			int specStart = i*leftSpec->getBinCount() / binCount,
				specEnd = (i+1)*leftSpec->getBinCount() / binCount;

			double leftDb = 0., rightDb = 0.;
			int leftCount = 0, rightCount = 0;
			for(int j = specStart; j < specEnd; ++j) {
				auto& leftBin = leftSpec->getByIndex(j);
				auto& rightBin = rightSpec->getByIndex(j);

				if(leftBin.getEnergy() >= leftAvgEnergy) {
					leftDb += leftBin.getEnergyDB();
					leftCount++;
				}
				if(rightBin.getEnergy() >= rightAvgEnergy) {
					rightDb += rightBin.getEnergyDB();
					rightCount++;
				}
			}
			if(leftCount > 0) {
				leftDb = (leftDb/leftCount + 60);
				if(leftSpec->getByIndex(i).getFreqStart() <= 150)
					leftDb += 10;
				if(leftDb < 0.)
					leftDb = 0.;
			}
			else {
				leftDb = 0.;
			}
			if(rightCount > 0) {
				rightDb = (rightDb/rightCount + 60);
				if(rightSpec->getByIndex(i).getFreqStart() <= 150)
					rightDb += 10;
				if(rightDb < 0.)
					rightDb = 0.;
			}
			else {
				rightDb = 0.;
			}

			
			double leftTop = leftDb / 60;
			if(leftTop > 1.)
				leftTop = 1.;
			
			leftHeights[i] = (leftTop >= leftHeights[i]) ? leftTop : leftTop*0.25 + leftHeights[i]*0.75;

			double rightTop = rightDb / 60;
			if(rightTop > 1.)
				rightTop = 1.;

			rightHeights[i] = (rightTop >= rightHeights[i]) ? rightTop : rightTop*0.25 + rightHeights[i]*0.75;


			//Additional gamma correction step
			double value = std::pow(leftHeights[i], 2.2);
			int half = strip.getSize() / 2;

			Color c = Color::HSV(i*240. / (binCount-1), 1., value);

			size_t ledStart = half - i*half/binCount,
				ledEnd = half - (i+1)*half/binCount;
			for(int j = ledStart; j > ledEnd; --j) {
				try {
				strip.setPixel(j, c);
				}
				catch(std::exception& e) {
					std::cout << "[left] i=" << i << ", j=" << j << std::endl;
					throw e;
				}
			}

			//Additional gamma correction step
			value = std::pow(rightHeights[i], 2.2);

			c = Color::HSV(i*240. / (binCount-1), 1., value);
			ledStart = half + i*half/binCount;
			ledEnd = half + (i+1)*half/binCount;
			for(int j = ledStart; j < ledEnd; ++j) {
				try {
				strip.setPixel(j, c);
				}
				catch(std::exception& e) {
					std::cout << "[right] i=" << i << std::endl;
					throw e;
				}
			}

		}
		node->releaseLightStrip();
	}

	leftAvgEnergy = 0.25*leftSpec->getAverageEnergy() + 0.75*leftAvgEnergy;
	rightAvgEnergy = 0.25*rightSpec->getAverageEnergy() + 0.75*rightAvgEnergy;

}
