#include "LightEffectStripEQ.hpp"

#include <cmath>
#include <iostream>
#include <algorithm>

#include "LightStripDigital.hpp"

LightEffectStripEQ::LightEffectStripEQ(
	std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer)
	:	ILightEffect({LightStrip::Type::Digital})
	,	spectrumAnalyzer(_spectrumAnalyzer)
	,	smoothed(spectrumAnalyzer->getLeftSpectrum()->getBinCount())
	,	avgEnergy{0.}{
	
}

void LightEffectStripEQ::tick() {
	const double NOISE_FLOOR = 60.;

	auto spec = spectrumAnalyzer->getLeftSpectrum();
	size_t binCount = spec->getBinCount();
	
	for(size_t i = 0; i < binCount; ++i) {
		auto bin = spec->getByIndex(i);
		double db = 0.;

		if(bin.getEnergy() >= avgEnergy) {
			db = bin.getEnergyDB() + NOISE_FLOOR;
		}
		
		//Bass boost
		if(bin.getFreqStart() <= 150)
			db += 10;
		if(db < 0.)
			db = 0.;
		
		//Convert to range [0,1]
		double top = db / 60;
		if(top > 1.)
			top = 1.;
		
		//Smooth with exponential filter
		smoothed[i] = (top >= smoothed[i]) ? top : top*0.25 + smoothed[i]*0.75;
	}

	//Smooth with exponential filter
	avgEnergy = 0.25*spec->getAverageEnergy() + 0.75*avgEnergy;
}

void LightEffectStripEQ::updateStrip(std::shared_ptr<LightStrip> strip) {
	auto buffer = LightBuffer_cast<LightBufferDigital>(strip->getBuffer());
	
	size_t ledCount = buffer->getSize(),
		binCount = smoothed.size();
	
	for(size_t i = 0; i < binCount; ++i) {
		Color c = Color::HSV(i*240./(binCount-1), 1., std::pow(smoothed[i], 2.2));
		
		size_t ledStart = i*ledCount/binCount,
			ledEnd = (i+1)*ledCount/binCount;
		for(size_t j = ledStart; j < ledEnd; ++j) {
			buffer->setColor(j, c);
		}
	}
}
