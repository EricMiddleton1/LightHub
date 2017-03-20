#include "LightEffectMatrixEQ.hpp"

#include <cmath>
#include <iostream>

#include "LightStripMatrix.hpp"

LightEffectMatrixEQ::LightEffectMatrixEQ(
	std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer, unsigned int _bandCount)
	:	ILightEffect({LightStrip::Type::Matrix})
	,	spectrumAnalyzer(_spectrumAnalyzer)
	,	bandCount{_bandCount}
	,	heights(bandCount) {
	
}

LightEffectMatrixEQ::~LightEffectMatrixEQ() {

}

void LightEffectMatrixEQ::tick() {
	auto leftSpec = spectrumAnalyzer->getLeftSpectrum();

	const double NOISE_FLOOR = 50.;

	for(int i = 0; i < bandCount; ++i) {
		int specStart = i*leftSpec->getBinCount() / bandCount,
			specEnd = (i+1)*leftSpec->getBinCount() / bandCount;

		double db = 0;
		for(int j = specStart; j < specEnd; ++j) {
			db += leftSpec->getByIndex(j).getEnergyDB();
		}
		db = (db/(specEnd - specStart) + NOISE_FLOOR) * 1.3;
		
		if(leftSpec->getByIndex(specStart).getFreqEnd() < 150.)
			db += 6;
		
		if(db < 0.)
			db = 0.;
		
		double top = db / NOISE_FLOOR;
		if(top > 1.)
			top = 1.;

		if(top > heights[i])
			heights[i] = top;
		else {
			heights[i] -= 0.05;
			if(heights[i] < top)
				heights[i] = top;
		}
	}
}

void LightEffectMatrixEQ::updateStrip(std::shared_ptr<LightStrip> strip) {
	auto buffer = LightBuffer_cast<LightBufferMatrix>(strip->getBuffer());
	
	buffer->setAll({});

	unsigned int width = 1, gap = 0, start = 0;
	if(buffer->getWidth() >= (2*bandCount - 1)) {
		gap = 1;

		width = std::ceil(buffer->getWidth() / (bandCount)) - 1;
		start = (buffer->getWidth() - (bandCount*width + gap*(bandCount-1))) / 2;
	}

	for(unsigned int i = 0; i < bandCount; ++i) {
		for(unsigned int j = 0; j < width; ++j) {
			unsigned int x = start + i*(gap+width) + j,
				top = heights[i]*(buffer->getHeight()-1) + 0.5;
			Color c = Color::HSV(240.*i/(bandCount-1), 1., 1.);

			for(unsigned int y = 0; y <= top; ++y) {
				buffer->setColor(x, buffer->getHeight() - y - 1, c);
			}
		}
	}
}
