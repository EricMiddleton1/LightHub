#include "SoundColor.hpp"

#include <algorithm>
#include <iostream>
#include <functional>
#include <sstream>

using namespace std;

std::string SoundColorSettings::toString() {
	std::stringstream ss;

	ss << "bassFreq = " << bassFreq << '\n'
		<< "trebbleFreq = " << trebbleFreq << '\n'
		<< "bassBoost = " << bassBoost << '\n'
		<< "trebbleBoost = " << trebbleBoost << '\n'
		<< "fStart = " << fStart << '\n'
		<< "fEnd = " << fEnd << '\n'
		<< "dbScaler = " << dbScaler << '\n'
		<< "dbFactor = " << dbFactor << '\n'
		<< "avgFactor = " << avgFactor << '\n'
		<< "noiseFloor = " << noiseFloor << '\n'
		<< "avgFilterStrength = " << avgFilterStrength << '\n'
		<< "minSaturation = " << minSaturation << '\n'
		<< "filterStrength = " << filterStrength << '\n'
		<< "centerSpread = " << centerSpread << '\n';
	
	return ss.str();
}


SoundColor::SoundColor(const SoundColorSettings& _settings)
	:	settings(_settings) {
}

Color SoundColor::getColor(Spectrum spectrum) {
	renderColor(spectrum);

	return c;
}

void SoundColor::renderColor(Spectrum& spectrum) {

	double r = 0., g = 0., b = 0.;
	size_t binCount = spectrum.getBinCount();

	double curAvg = spectrum.getAverageEnergyDB() + settings.noiseFloor;

	if(curAvg < 0)
		curAvg = 0;

	double absFloor = std::pow(10., -100);
	double absBoost = std::pow(10., settings.bassBoost/20.);
	
	avg = avg*settings.avgFilterStrength
		+ curAvg*(1. - settings.avgFilterStrength);

	//Scale to be applied to each bin
	double scale = 1. / settings.dbScaler;

	bool isBass = true;
	unsigned int bassIndex = 0;

	for(unsigned int i = 0; i < binCount; ++i) {
		FrequencyBin& bin = spectrum.getByIndex(i);
		double f = bin.getFreqCenter();

		if(f > settings.fEnd)
			break;

		if(f >= settings.fStart) {
			if(isBass && f > settings.bassFreq) {
				bassIndex = i;
				isBass = false;
			}
			
			float hue = (isBass) ? 0.f : (240.f * (i-bassIndex) / (binCount - bassIndex - 1));

			Color c = Color::HSV(hue, 1.f, 1.f);
			double db = bin.getEnergyDB();
			double energy = bin.getEnergy();

			//Bass boost
			if(f <= settings.bassFreq)
			db += settings.bassBoost;

			//Trebble boost
			if(f >= settings.trebbleFreq)
				db += settings.trebbleBoost;
			
			//Raise by noise floor, subtract loosly-tracking average
			db += settings.noiseFloor - avg;

			//Reject anything below the average
			if(db < 0)
				continue;

			//Scale partially based on average level
			db *= settings.dbFactor;
			db += settings.avgFactor*avg;

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
	if(largest > 255) {
		double scale = 255. / largest;

		r *= scale;
		g *= scale;
		b *= scale;
	}

	Color cTmp(r, g, b);
	double h = cTmp.getHue(), s = cTmp.getHSVSaturation(), v = cTmp.getValue();

	//Enforce saturation minimum
	cTmp = Color::HSV(h, std::max(settings.minSaturation, s), v);

	//Filter the color
	c.filter(cTmp, settings.filterStrength);
}
