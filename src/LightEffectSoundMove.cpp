#include "LightEffectSoundMove.hpp"
#include "LightStripDigital.hpp"

#include <cmath>
#include <iostream>

LightEffectSoundMove::LightEffectSoundMove(
	std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer)
	:	ILightEffect({LightStrip::Type::Digital})
	,	spectrumAnalyzer(_spectrumAnalyzer) {
	
}

void LightEffectSoundMove::tick() {
	const int nBands = 3;
	const double minBright = 64, filterStrength = 0.9, bassBoost = 4., scale = 10.;
	static std::vector<double> bandFilter(nBands);

	auto spec = spectrumAnalyzer->getLeftSpectrum();
	double r = 0., g = 0., b = 0.;

	for(int i = 0; i < nBands; ++i) {
		int start = spec.getBinCount() * i / nBands,
			end = spec.getBinCount()*(i+1) / nBands - 1;
		double sum = 0.;

		for(int j = start; j < end; ++j) {
			sum += spec.getByIndex(j).getEnergy();
		}

		sum /= end - start;
		if(i == 0)
			sum *= bassBoost;

		bandFilter[i] = filterStrength*bandFilter[i] + (1. - filterStrength)*sum;

		double highPass = sum - bandFilter[i];
		
		if(highPass < 0)
			continue;

		Color c = Color::HSV(240.*i / (nBands - 1), 1., 1.);
		r += highPass * c.getRed();
		g += highPass * c.getGreen();
		b += highPass * c.getBlue();
	}
	
	r *= scale;
	g *= scale;
	b *= scale;

	if(r < minBright)
		r = 0;
	if(g < minBright)
		g = 0;
	if(b < minBright)
		b = 0;

	double max = r;
	if(g > max)
		max = g;
	if(b > max)
		max = b;
	
	if(max > 255.) {
		double scale = 255. / max;

		r *= scale;
		g *= scale;
		b *= scale;
	}

	curColor.filter({(int)(r + 0.5), (int)(g + 0.5), (int)(b + 0.5)}, 0.5);
}

void LightEffectSoundMove::updateStrip(std::shared_ptr<LightStrip> strip) {
	auto buffer = LightBuffer_cast<LightBufferDigital>(strip->getBuffer());

	*buffer << 1;
	buffer->setColor(0, curColor);
}
