#include "LightEffectStripEQ.hpp"

#include <cmath>
#include <iostream>
#include <algorithm>

#include "LightStripDigital.hpp"

LightEffectStripEQ::LightEffectStripEQ(
	std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer)
	:	LightEffect({LightStrip::Type::Digital}, {{"reverse", false}})
	,	spectrumAnalyzer(_spectrumAnalyzer)
	,	smoothed(spectrumAnalyzer->getLeftSpectrum().getBinCount())
	,	avgEnergy{0.} {
}

void LightEffectStripEQ::tick() {
	const double NOISE_FLOOR = 50.;

	Spectrum spec = spectrumAnalyzer->getMonoSpectrum();
	size_t binCount = spec.getBinCount();

	double maxVal = 0.;

	for(size_t i = 0; i < binCount; ++i) {
		auto bin = spec.getByIndex(i);
		double db = 0.;

		if(bin.getEnergy() >= avgEnergy) {
			db = bin.getEnergyDB() + NOISE_FLOOR;
		}

		//Convert to range [0,1]
		double top = 1.4*(db / NOISE_FLOOR);
		if(top > 1.)
			top = 1.;

		//Smooth with exponential filter
		smoothed[i] = (top >= smoothed[i]) ? top : top*0.1 + smoothed[i]*0.9;

		if(smoothed[i] > maxVal) {
			maxVal = smoothed[i];
		}
	}

	//Smooth with exponential filter
	avgEnergy = 0.25*spec.getAverageEnergy() + 0.75*avgEnergy;

	//std::cout << maxVal << std::endl;
}

void LightEffectStripEQ::updateStrip(std::shared_ptr<LightStrip> strip) {
	bool reverse = getParameter("reverse").getValue().getBool();
	
	auto buffer = LightBuffer_cast<LightBufferDigital>(strip->getBuffer());

	size_t ledCount = buffer->getSize(),
		binCount = smoothed.size();
	
	for(size_t i = 0; i < binCount; ++i) {
		Color c = Color::HSV(i*240./(binCount-1), 1., smoothed[i]);
		c.gammaCorrect(2.2);

		size_t ledStart = i*ledCount/binCount,
			ledEnd = (i+1)*ledCount/binCount;
		for(size_t j = ledStart; j < ledEnd; ++j) {
			unsigned int x = reverse ? (ledCount - j - 1) : j;
			buffer->setColor(x, c);
		}
	}
}
