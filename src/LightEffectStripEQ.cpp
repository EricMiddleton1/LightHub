#include "LightEffectStripEQ.hpp"

#include <cmath>
#include <iostream>
#include <algorithm>

LightEffectStripEQ::LightEffectStripEQ(
	std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer, size_t _maxBins)
	:	ILightEffect({LightNode::Type::DIGITAL})
	,	spectrumAnalyzer(_spectrumAnalyzer)
	,	maxBins{_maxBins}
	,	avgEnergy{0.}{
	
}

LightEffectStripEQ::~LightEffectStripEQ() {

}

void LightEffectStripEQ::addNode(const std::shared_ptr<LightNode>& node) {
	ILightEffect::addNode(node);

	size_t binCount = std::min((size_t)node->getWidth(), maxBins);

	values.insert({node, std::vector<double>(binCount)});
}

void LightEffectStripEQ::removeNode(std::shared_ptr<LightNode> node) {
	ILightEffect::removeNode(node);

	values.erase(node);
}

void LightEffectStripEQ::update() {
	auto leftSpec = spectrumAnalyzer->getRightSpectrum();

	for(auto& node : nodes) {
		auto& heights = values.at(node);
		size_t binCount = heights.size();
		
		auto& strip = node->getLightStrip();
		strip.setAll({});

		for(int i = 0; i < binCount; ++i) {
			int specStart = i*leftSpec->getBinCount() / binCount,
				specEnd = (i+1)*leftSpec->getBinCount() / binCount;

			double db = 0;
			int count = 0;
			for(int j = specStart; j < specEnd; ++j) {
				auto& bin = leftSpec->getByIndex(j);

				if(bin.getEnergy() >= avgEnergy) {
					db += bin.getEnergyDB();
					count++;
				}
			}
			if(count > 0) {
				db = (db/count + 60);
				if(leftSpec->getByIndex(i).getFreqStart() <= 150)
					db += 10;
				if(db < 0.)
					db = 0.;
			}
			else {
				db = 0.;
			}
			
			double top = db / 60;
			if(top > 1.)
				top = 1.;
			
			heights[i] = (top >= heights[i]) ? top : top*0.25 + heights[i]*0.75;

			//Additional gamma correction step
			double value = std::pow(heights[i], 2.2);

			Color c = Color::HSV(i*240. / (binCount-1), 1., value);
			size_t ledStart = i*strip.getSize() / binCount,
				ledEnd = (i+1)*strip.getSize() / binCount;
			for(size_t j = ledStart; j < ledEnd; ++j) {
				strip.setPixel(j, c);
			}
		}
		node->releaseLightStrip();
	}

	avgEnergy = 0.25*leftSpec->getAverageEnergy() + 0.75*avgEnergy;
}
