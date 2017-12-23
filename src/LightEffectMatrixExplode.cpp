#include "LightEffectMatrixExplode.hpp"

#include <cmath>
#include <iostream>
#include <algorithm>

#include "Matrix.hpp"

LightEffectMatrixExplode::LightEffectMatrixExplode(
	std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer)
	:	LightEffect({{"bass freq", 150.}, {"trebble freq", 4000.}, {"bass boost", 12.},
			{"trebble boost", 0.}, {"start frequency", 0.}, {"end frequency", 20000.},
			{"db scaler", 51200.}, {"db factor", 1.}, {"average factor", 0.5},
			{"change factor", 0.},
			{"noise floor", 50.}, {"average filter strength", 0.4}, {"min saturation", 0.6},
			{"color filter strength", 0.8}, {"threshold", 0.},
			{"invert", false}, {"multiplier", 1.1}, {"band count",
			(double)_spectrumAnalyzer->getLeftSpectrum().getBinCount(),
			Parameter::ValidatorRange(1., _spectrumAnalyzer->getLeftSpectrum().getBinCount())}})
	,	spectrumAnalyzer(_spectrumAnalyzer)
	,	colors{Color(0, 0, 0)} {
	
}

LightEffectMatrixExplode::~LightEffectMatrixExplode() {
}

bool LightEffectMatrixExplode::validateLight(const std::shared_ptr<Light>& light) {
	return true;
}

void LightEffectMatrixExplode::tick() {
	static int tick = 0;

	double maxRadius = std::sqrt(32*32 + 24*24);

	if(tick == 0) {
		Spectrum spec = spectrumAnalyzer->getMonoSpectrum();
	
		colors.insert(colors.begin(), renderColor(colors.front(), spec));
		if(colors.size() >= maxRadius) {
			colors.erase(colors.end());
		}
	}

	tick = (tick + 1) % 2;
}

void LightEffectMatrixExplode::updateLight(std::shared_ptr<Light>& light) {
	auto multiplier = getParameter("multiplier").getValue().getDouble();
	bool invert = getParameter("invert").getValue().getBool();

	auto buffer = buffer_cast<MatrixBuffer>(light->getBuffer());
	auto matrixWidth = buffer->getWidth();
	auto matrixHeight = buffer->getHeight();
	
	buffer->setAll({});
	
	int midX = matrixWidth/2, midY = matrixHeight/2;

	for(int y = 0; y < matrixHeight; ++y) {
		for(int x = 0; x < matrixWidth; ++x) {
			int normX = x-midX, normY = y-midY;
			double r = std::sqrt(normX*normX + normY*normY);
			double theta = std::atan2(normY, normX) + M_PI;

			int bin = r;

			if(bin < colors.size()) {
				buffer->set(x, y, colors[bin]);
			}
		}
	}
}

Color LightEffectMatrixExplode::renderColor(const Color& prev, Spectrum spectrum) {
	double bassFreq = getParameter("bass freq").getValue().getDouble();
	double trebbleFreq = getParameter("trebble freq").getValue().getDouble();
	double bassBoost = getParameter("bass boost").getValue().getDouble();
	double trebbleBoost = getParameter("trebble boost").getValue().getDouble();
	double fStart = getParameter("start frequency").getValue().getDouble();
	double fEnd = getParameter("end frequency").getValue().getDouble();
	double dbScaler = getParameter("db scaler").getValue().getDouble();
	double dbFactor = getParameter("db factor").getValue().getDouble();
	double avgFactor = getParameter("average factor").getValue().getDouble();
	double changeFactor = getParameter("change factor").getValue().getDouble();
	double noiseFloor = getParameter("noise floor").getValue().getDouble();
	double avgFilterStrength = getParameter("average filter strength").getValue().getDouble();
	uint8_t minSaturation = getParameter("min saturation").getValue().getDouble()*255;
	double filterStrength = getParameter("color filter strength").getValue().getDouble();
	uint8_t threshold = getParameter("threshold").getValue().getDouble()*255;

	static int bassIndex = -1, endIndex;
	static double avg = 0.;

	double r = 0., g = 0., b = 0.;
	size_t binCount = spectrum.getBinCount();

	Color c = prev;

	if(bassIndex == -1) {
		for(bassIndex = 0; (bassIndex < binCount) &&
			(spectrum.getByIndex(bassIndex).getFreqCenter() <= bassFreq); ++bassIndex);
		
		for(endIndex = 0; (endIndex < binCount) &&
			(spectrum.getByIndex(endIndex).getFreqCenter() <= fEnd); ++endIndex);
	}

	//Compute average
	double curAvg = 0;
	for(unsigned int i = 0; i < endIndex; ++i) {
		curAvg += spectrum.getByIndex(i).getEnergy();
	}

	curAvg = 20.*std::log10(curAvg/endIndex) + noiseFloor;

	//double curAvg = spectrum.getAverageEnergyDB() + noiseFloor;

	if(curAvg < 0)
		curAvg = 0;

	avg = avg*avgFilterStrength
		+ curAvg*(1. - avgFilterStrength);

	//Scale to be applied to each bin
	double scale = 1. / dbScaler;

	for(unsigned int i = 0; i < binCount; ++i) {
		FrequencyBin& bin = spectrum.getByIndex(i);
		double f = bin.getFreqCenter();

		if(f > fEnd)
			break;

		if(f >= fStart) {
			//float hue = (f <= bassFreq) ? 40.f*std::pow((double)i/(bassIndex-1), 4.)
				//: (45.f + 240.f * (i-bassIndex) / (binCount - bassIndex - 1));
			//float hue = 240. * i / (binCount - 1);
			int yellowPoint = 11;

			
			float hue;
			if(i < yellowPoint) {
				hue = 60. * i / (yellowPoint-1);
			}
			else {
				hue = 60. + 180.*(i-yellowPoint) / (binCount - yellowPoint - 1);
			}

			hue = 240.f * i / (binCount - 1);
			hue = std::floor(i * 3 / binCount) * 120.f;

			Color c = Color::HSV(255.f*hue/360.f, 255, 255);

			double db = bin.getEnergyDB();
			
			//Bass boost
			if(f <= bassFreq)
				db += bassBoost;

			//Trebble boost
			if(f >= trebbleFreq)
				db += trebbleBoost;
			
			//Raise by noise floor, subtract loosly-tracking average
			db += noiseFloor - avg;

			//Reject anything below the average
			if(db < 0) {
				continue;
			}

			//Scale partially based on average level
			db *= dbFactor;
			db += avgFactor*avg;

			r += db * c.getRed();
			g += db * c.getGreen();
			b += db * c.getBlue();
		}
	}

	//Scale color
	r *= scale;
	g *= scale;
	b *= scale;

	//Compute largest component
	double largest = std::max(r, std::max(g, b));

	//Use the largest value to limit the maximum brightness to 255
	if(largest > 1.f) {
		double scale = 1.f / largest;

		r *= scale;
		g *= scale;
		b *= scale;
	}

	r = 255*std::pow(r, 2.2f);
	g = 255*std::pow(g, 2.2f);
	b = 255*std::pow(b, 2.2f);

	Color cTmp(r, g, b);
	
	uint8_t h = cTmp.getHue(), s = cTmp.getSat(), v = cTmp.getVal();

	if(v < threshold) {
		v = 0;
	}

	//Enforce saturation minimum
	cTmp = Color::HSV(h, std::max(minSaturation, s), v);

	//Filter the color
	c.filter(cTmp, filterStrength);

	return c;
}
