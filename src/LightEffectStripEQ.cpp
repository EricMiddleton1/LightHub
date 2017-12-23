#include "LightEffectStripEQ.hpp"

#include <cmath>
#include <iostream>
#include <algorithm>

LightEffectStripEQ::LightEffectStripEQ(
	std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer)
	:	LightEffect{{{"reverse", false}, {"smooth", false}, {"stereo", false},
			{"multiplier", 1.4}}}
	,	spectrumAnalyzer(_spectrumAnalyzer)
	,	leftSmoothed(spectrumAnalyzer->getLeftSpectrum().getBinCount())
	,	rightSmoothed(spectrumAnalyzer->getLeftSpectrum().getBinCount())
	,	monoSmoothed(spectrumAnalyzer->getLeftSpectrum().getBinCount())
	,	leftAvg{0.}
	,	rightAvg{0.}
	,	monoAvg{0.} {
}

bool LightEffectStripEQ::validateLight(const std::shared_ptr<Light>& light) {
	return light->getSize() > 1;
}

void LightEffectStripEQ::tick() {
	smoothSpectrum(leftSmoothed, leftAvg, spectrumAnalyzer->getLeftSpectrum());
	smoothSpectrum(rightSmoothed, rightAvg, spectrumAnalyzer->getRightSpectrum());
	smoothSpectrum(monoSmoothed, monoAvg, spectrumAnalyzer->getMonoSpectrum());
}

void LightEffectStripEQ::smoothSpectrum(std::vector<double>& smoothed, double& avgEnergy,
	const Spectrum& spec) {
	
	const double NOISE_FLOOR = 50.;
	
	double multiplier = getParameter("multiplier").getValue().getDouble();

	size_t binCount = spec.getBinCount();

	for(size_t i = 0; i < binCount; ++i) {
		auto bin = spec.getByIndex(i);
		double db = 0.;

		if(bin.getEnergy() >= avgEnergy) {
			db = bin.getEnergyDB() + NOISE_FLOOR;
		}

		//Convert to range [0,1]
		double top = multiplier*(db / NOISE_FLOOR);
		if(top > 1.)
			top = 1.;

		//Smooth with exponential filter
		smoothed[i] = (top >= smoothed[i]) ? top : top*0.1 + smoothed[i]*0.9;

		smoothed[i] = std::max(0., std::min(1., smoothed[i]));
	}

	//Smooth with exponential filter
	avgEnergy = 0.25*spec.getAverageEnergy() + 0.75*avgEnergy;
}


float gammaCorrect(float value, float gamma) {
	return std::pow(value, gamma);
}

void LightEffectStripEQ::updateLight(std::shared_ptr<Light>& light) {
	bool reverse = getParameter("reverse").getValue().getBool();
	bool smooth = getParameter("smooth").getValue().getBool();
	bool stereo = getParameter("stereo").getValue().getBool();

	
	auto buffer = light->getBuffer();

	size_t ledCount = buffer->getSize(),
		binCount = leftSmoothed.size();
	
	if(smooth) {
		if(stereo) {
			for(size_t i = 0; i < ledCount/2; ++i) {
				float bin = static_cast<float>(i)*binCount / (ledCount/2);
				size_t first = static_cast<size_t>(bin),
					second = first + 1;
				float frac = bin - first;

				float v = gammaCorrect(leftSmoothed[second], 2.2f)*frac +
					gammaCorrect(leftSmoothed[first], 2.2f)*(1.f-frac);
				v = std::min(1.f, std::max(0.f, v));

				float hue = 240.f*i/(ledCount/2*360.f);

				size_t ledIndex = reverse ? (ledCount - (ledCount/2-i-1) - 1) : (ledCount/2-i-1);

				if(v < 0.05f) {
					v = 0.f;
				}
				else if( v < 0.15f) {
					v = 0.15f;
				}

				Color c = Color::HSV(255.f*hue, 255, 255.f*v);
				//c.gammaCorrect(2.2);
				buffer->at(ledIndex) = c;
			}
			for(size_t i = 0; i < ledCount/2; ++i) {
				float bin = static_cast<float>(i)*binCount / (ledCount/2);
				size_t first = static_cast<size_t>(bin),
					second = first + 1;
				float frac = bin - first;

				float v = gammaCorrect(rightSmoothed[second], 2.2f)*frac +
					gammaCorrect(rightSmoothed[first], 2.2f)*(1.f-frac);
				v = std::min(1.f, std::max(0.f, v));

				float hue = 240.f*i/(ledCount/2*360.f);

				size_t ledIndex = reverse ? (ledCount/2 - i - 1) : i + ledCount/2;

				if(v < 0.05f) {
					v = 0.f;
				}
				else if( v < 0.15f) {
					v = 0.15f;
				}

				Color c = Color::HSV(255.f*hue, 255, 255.f*v);
				//c.gammaCorrect(2.2);
				buffer->at(ledIndex) = c;
			}
		}
		else {
			for(size_t i = 0; i < ledCount; ++i) {
				float bin = static_cast<float>(i)*binCount / ledCount;
				size_t first = static_cast<size_t>(bin),
					second = first + 1;
				float frac = bin - first;

				float v = gammaCorrect(monoSmoothed[second], 2.2f)*frac +
					gammaCorrect(monoSmoothed[first], 2.2f)*(1.f-frac);
				v = std::min(1.f, std::max(0.f, v));

				float hue = 240.f*i/(ledCount*360.f);

				size_t ledIndex = reverse ? (ledCount - i - 1) : i;

				if(v < 0.05f) {
					v = 0.f;
				}
				else if( v < 0.15f) {
					v = 0.15f;
				}

				Color c = Color::HSV(255.f*hue, 255, 255.f*v);
				//c.gammaCorrect(2.2);
				buffer->at(ledIndex) = c;
			}
		}
	}
	else {
		for(size_t i = 0; i < binCount; ++i) {
			float h = 255.f*i*240./(binCount-1)/360.f, s = 255.f, v = 255.f*monoSmoothed[i];

			if(v < 0) {
				v = 0;
			}
			else if(v > 255) {
				v = 255;
			}

			Color c = Color::HSV(h, s, v);
			c.gammaCorrect(2.2);

			size_t ledStart = i*ledCount/binCount,
				ledEnd = (i+1)*ledCount/binCount;
			for(size_t j = ledStart; j < ledEnd; ++j) {
				unsigned int x = reverse ? (ledCount - j - 1) : j;
				buffer->at(x) = c;
			}
		}
	}
}
