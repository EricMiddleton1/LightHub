#include "LightEffectMatrixEQ.hpp"

#include <cmath>
#include <iostream>

LightEffectMatrixEQ::LightEffectMatrixEQ(
	std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer)
	:	ILightEffect({LightNode::Type::MATRIX})
	,	spectrumAnalyzer(_spectrumAnalyzer) {
	
}

LightEffectMatrixEQ::~LightEffectMatrixEQ() {

}

void LightEffectMatrixEQ::addNode(const std::shared_ptr<LightNode>& node) {
	ILightEffect::addNode(node);

	values.insert({node, std::vector<double>(node->getWidth())});
}

void LightEffectMatrixEQ::removeNode(std::shared_ptr<LightNode> node) {
	ILightEffect::removeNode(node);

	values.erase(node);
}

void LightEffectMatrixEQ::update() {
	auto leftSpec = spectrumAnalyzer->getLeftSpectrum();

	for(auto& node : nodes) {
		auto& heights = values.at(node);
		int width = node->getWidth(), height = node->getHeight();
		
		auto& strip = node->getLightStrip();
		strip.setAll({});

		for(int i = 0; i < width; ++i) {
			int specStart = i*leftSpec->getBinCount() / width,
				specEnd = (i+1)*leftSpec->getBinCount() / width - 1;

			double db = 0;
			for(int j = specStart; j < specEnd; ++j) {
				db += leftSpec->getByIndex(j).getEnergyDB();
			}
			db = (db/(specEnd - specStart) + 50) * 1.3;
			if(i < 2)
				db += 10;
			if(db < 0.)
				db = 0.;
			
			int top = db * height / 50;
			if(top > height)
				top = height;

			if(top > heights[i])
				heights[i] = top;
			else {
				heights[i] -= 0.5;
				if(heights[i] < top)
					heights[i] = top;
			}

			top = heights[i] + 0.5;

			for(int y = 0; y < top; ++y) {
				strip.setPixel((height - y - 1)*width + i,
					Color::HSV(i*240. / (width-1), 1., 0.4));
			}
		}
		node->releaseLightStrip();
	}
}
