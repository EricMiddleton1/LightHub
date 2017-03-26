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
		<< "threshold = " << threshold << '\n';
	
	return ss.str();
}


SoundColor::SoundColor(const SoundColorSettings& _settings)
	:	settings(_settings) {
}

Color SoundColor::getColor(Spectrum spectrum) {
	renderColor(spectrum);
/*
	if(settings.fEnd > 200.) {
		std::cout << c.toString() << std::endl;
	}
*/

	return c;
}

void SoundColor::renderColor(Spectrum& spectrum) {
	static int bassIndex = -1, endIndex = -1;

	double r = 0., g = 0., b = 0.;
	size_t binCount = spectrum.getBinCount();

	if(bassIndex == -1) {
		for(bassIndex = 0; (bassIndex < binCount) &&
			(spectrum.getByIndex(bassIndex).getFreqCenter() <= settings.bassFreq); ++bassIndex);
		std::cout << "Bass Index: " << bassIndex << std::endl;
	}
	if(endIndex == -1) {
		for(endIndex = 0; (endIndex < binCount) &&
			(spectrum.getByIndex(endIndex).getFreqCenter() <= settings.fEnd); ++endIndex);
		std::cout << "End Index: " << endIndex << std::endl;
	}

	//Compute average
	double curAvg = 0;
	for(unsigned int i = 0; i < endIndex; ++i) {
		curAvg += spectrum.getByIndex(i).getEnergy();
	}
	curAvg = 20.*std::log10(curAvg/endIndex) + settings.noiseFloor;

	//double curAvg = spectrum.getAverageEnergyDB() + settings.noiseFloor;

	if(curAvg < 0)
		curAvg = 0;

	avg = avg*settings.avgFilterStrength
		+ curAvg*(1. - settings.avgFilterStrength);

	//Scale to be applied to each bin
	double scale = 1. / settings.dbScaler;

	for(unsigned int i = 0; i < binCount; ++i) {
		FrequencyBin& bin = spectrum.getByIndex(i);
		double f = bin.getFreqCenter();

		if(f > settings.fEnd)
			break;

		if(f >= settings.fStart) {
			float hue = (f <= settings.bassFreq) ? 40.f*std::pow((double)i/(bassIndex-1), 4.)
				: (45.f + 240.f * (i-bassIndex) / (binCount - bassIndex - 1));

			Color c = Color::HSV(hue, 1.f, 1.f);
			double db = bin.getEnergyDB();

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

	if(v < settings.threshold) {
		v = 0.;
	}

	//Enforce saturation minimum
	cTmp = Color::HSV(h, std::max(settings.minSaturation, s), v);

	//Filter the color
	c.filter(cTmp, settings.filterStrength);
}
