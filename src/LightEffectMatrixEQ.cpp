#include "LightEffectMatrixEQ.hpp"

#include <cmath>
#include <iostream>
#include <algorithm>

#include "LightStripMatrix.hpp"

LightEffectMatrixEQ::LightEffectMatrixEQ(
	std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer)
	:	LightEffect({LightStrip::Type::Matrix},
		{{"invert", false}, {"band count",
			(double)_spectrumAnalyzer->getLeftSpectrum().getBinCount(),
			Parameter::ValidatorRange(1., _spectrumAnalyzer->getLeftSpectrum().getBinCount())}})
	,	spectrumAnalyzer(_spectrumAnalyzer)
	,	heights(getParameter("band count").getValue().getNumber()) {
	
}

LightEffectMatrixEQ::~LightEffectMatrixEQ() {

}

void LightEffectMatrixEQ::tick() {
	Spectrum spec = spectrumAnalyzer->getMonoSpectrum();

	const double MIN_FLOOR = 50.;

	unsigned int bandCount = getParameter("band count").getValue().getNumber();

	if(bandCount != heights.size()) {
		heights = std::vector<double>(bandCount);
	}

	std::vector<FrequencyBin> sorted(spec.begin(), spec.end());
	std::sort(sorted.begin(), sorted.end(),
		[](const FrequencyBin& first, const FrequencyBin& second) {
			return (first.getEnergy() < second.getEnergy());
		});
	
	double sum = 0.;
	std::for_each(sorted.begin() + sorted.size()/4, sorted.end(),
		[&sum](const FrequencyBin& bin) {
			sum += bin.getEnergyDB();
		});

	double noiseFloor = MIN_FLOOR;//std::min(MIN_FLOOR, -(sum/(sorted.size()*3/4))*1.5);

	for(int i = 0; i < bandCount; ++i) {
		int specStart = i*spec.getBinCount() / bandCount,
			specEnd = (i+1)*spec.getBinCount() / bandCount;

		double db = 0;
		for(int j = specStart; j < specEnd; ++j) {
			db += spec.getByIndex(j).getEnergyDB();
		}
		db = (db/(specEnd - specStart) + noiseFloor) * 1.3;
		
		if(spec.getByIndex(specStart).getFreqEnd() < 150.)
			db += 6;
		
		if(db < 0.)
			db = 0.;
		
		double top = db / noiseFloor;
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

	std::vector<double> bars;

	unsigned int bandCount = heights.size();
	bool invert = getParameter("invert").getValue().getBool();

	unsigned int width = 1, gap = 0, start = 0;
	if(buffer->getWidth() >= (2*bandCount - 1)) {
		//gap = 1;

		//Interpolate between bars
		for(unsigned int i = 0; i < (heights.size() - 1); ++i) {
			bars.push_back(heights[i]);
			bars.push_back((heights[i] + heights[i+1])/2.);
		}
		bars.push_back(heights[heights.size()-1]);

		width = std::ceil(buffer->getWidth() / (bars.size())) - gap;
	}
	else {
		bars = heights;
	}
	start = (buffer->getWidth() - (bars.size()*width + gap*(bars.size()-1))) / 2;


	for(unsigned int i = 0; i < bars.size(); ++i) {
		for(unsigned int j = 0; j < width; ++j) {
			unsigned int x = start + i*(gap+width) + j,
				top = bars[i]*(buffer->getHeight()-1) + 0.5;
			float hue = 240.*i/(bars.size()-1);

			for(unsigned int y = 0; y <= top; ++y) {
				unsigned int yPos = (invert) ? (y) : (buffer->getHeight() - y - 1);

				buffer->setColor(x, yPos,
					Color::HSV(hue, 1.f, std::pow(1.f - 1.0f*y / top, 1.0)));
			}
		}
	}
}
