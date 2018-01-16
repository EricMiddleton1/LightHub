#include "LightEffectMatrixCircEQ.hpp"

#include <cmath>
#include <iostream>
#include <algorithm>

#include "Matrix.hpp"

LightEffectMatrixCircEQ::LightEffectMatrixCircEQ(
	std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer)
	:	LightEffect({{"invert", false}, {"multiplier", 1.1}, {"band count",
			(double)_spectrumAnalyzer->getLeftSpectrum().getBinCount(),
			Parameter::ValidatorRange(1., _spectrumAnalyzer->getLeftSpectrum().getBinCount())}})
	,	spectrumAnalyzer(_spectrumAnalyzer)
	,	heights(getParameter("band count").getValue().getDouble()) {
	
}

LightEffectMatrixCircEQ::~LightEffectMatrixCircEQ() {
}

bool LightEffectMatrixCircEQ::validateLight(const std::shared_ptr<Light>& light) {
	return true;
}

void LightEffectMatrixCircEQ::tick() {
	Spectrum spec = spectrumAnalyzer->getMonoSpectrum();

	const double MIN_FLOOR = 50.;

	unsigned int bandCount = getParameter("band count").getValue().getDouble();

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
			db += spec.getByIndex(j).getEnergy();
		}
		db = (db/(specEnd - specStart));
		db = 20. * std::log10(db) + noiseFloor; 
		
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
			heights[i] -= 0.025;
			if(heights[i] < top)
				heights[i] = top;
		}
	}
}

void LightEffectMatrixCircEQ::updateLight(std::shared_ptr<Light>& light) {
	auto multiplier = getParameter("multiplier").getValue().getDouble();

	auto buffer = buffer_cast<MatrixBuffer>(light->getBuffer());
	auto matrixWidth = buffer->getWidth();
	auto matrixHeight = buffer->getHeight();
	
	buffer->setAll({});

	std::vector<double> bars = heights;

	unsigned int bandCount = heights.size();
	bool invert = getParameter("invert").getValue().getBool();
	
	int width = std::ceil(matrixWidth / (bars.size()));
	int start = (matrixWidth - bars.size()*width) / 2;

	int midX = matrixWidth/2, midY = matrixHeight/2;
	double rMax = std::sqrt(midX*midX + midY*midY);

	for(int y = 0; y < matrixHeight; ++y) {
		for(int x = 0; x < matrixWidth; ++x) {
			int normX = x-midX, normY = y-midY;
			double r = std::sqrt(normX*normX + normY*normY)/rMax;
			if(r > 0) {
				double theta = std::atan2(normY, normX) + M_PI;

				int bin = theta*bandCount / (2*M_PI);
				if(bin == bandCount) {
					bin = 0;
				}
				
				float hueBinFrac = theta*bandCount / (2*M_PI);
				if(hueBinFrac >= bandCount) {
					hueBinFrac -= bandCount;
				}
				int hueBin = hueBinFrac;
				hueBinFrac -= hueBin;

				double alpha = 1.f - std::pow(std::abs(hueBinFrac - 0.5f), 2.f);
				alpha = std::pow(alpha, 5.);

				float hue = 300.*hueBin/(bandCount-1);

				if(bars[bin]*multiplier >= r) {
					buffer->set(x, y, Color::HSV(hue*255.f/360.f, 255, alpha * 255));
				}
			}
		}
	}
}
