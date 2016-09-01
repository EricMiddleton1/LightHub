#include "SoundColor.hpp"

#include <algorithm>
#include <iostream>
#include <functional>
#include <sstream>


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
		<< "slopeLimitAvg = " << slopeLimitAvg << '\n'
		<< "minSaturation = " << minSaturation << '\n'
		<< "filterStrength = " << filterStrength << '\n'
		<< "centerSpread = " << centerSpread << '\n';
	
	return ss.str();
}


SoundColor::SoundColor(std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer,
	const SoundColorSettings& _settings)
	:	settings(_settings)
	,	spectrumAnalyzer{_spectrumAnalyzer}
	,	hasChanged{false} {

	std::cout << settings.toString() << std::endl;

	//Get the spectrum to calculate the color vector
	auto spectrum = spectrumAnalyzer->getLeftSpectrum();

	//Determine the corresponding bin to freqBass
	size_t bassBin = 0;
	for(; bassBin < spectrum->getBinCount(); ++bassBin) {
		auto& bin = spectrum->getByIndex(bassBin);

		if(bin.getFreqCenter() >= settings.bassFreq)
			break;
	}

	//bassBin minimum of 2
	if(bassBin < 2)
		bassBin = 2;

	for(size_t i = 0; i < bassBin; i++) {
		double hue = i * 30. / (bassBin - 2);

		Color c = Color::HSV(hue, 1., 1.);

		frequencyColors.push_back(c);

		std::cout << hue << ":\t" << c.toString() << '\n';
	}

	for(size_t i = bassBin; i < spectrum->getBinCount(); i++) {
		double hue = 30. + (i - bassBin) * 210. /
			(spectrum->getBinCount() - bassBin - 1);

		Color c = Color::HSV(hue, 1., 1.);

		frequencyColors.push_back(c);

		std::cout << hue << ":\t" << c.toString() << '\n';
	}

	std::cout << std::endl;

	//Initialize each color channel
	left.avg = 0.;
	center.avg = 0.;
	right.avg = 0.;

	//spectrumAnalyzer->addListener(std::bind(SoundColor::cbSpectrum, this));
	spectrumAnalyzer->addListener([this](SpectrumAnalyzer* sa,
		std::shared_ptr<Spectrum> left, std::shared_ptr<Spectrum> right) {
		cbSpectrum(sa, left, right);
	});
}

SoundColor::~SoundColor() {
	//Remove the spectrum listener
	//spectrumAnalyzer->removeListener(std::bind(&SoundColor::cbSpectrum, this));
}

bool SoundColor::changed() {
	return hasChanged;
}

void SoundColor::getColor(Color* _left, Color* _center, Color* _right) {
	*_left = left.c;
	*_center = center.c;
	*_right = right.c;

	hasChanged = false;
}

void SoundColor::cbSpectrum(SpectrumAnalyzer*,
	std::shared_ptr<Spectrum> leftSpectrum,
	std::shared_ptr<Spectrum> rightSpectrum) {

	//Render color for left, right channels
	renderColor(left, *leftSpectrum.get());
	//renderColor(right, *rightSpectrum.get());

	//std::cout << left.c.toString() << std::endl;

	//TODO: render color for center channel

	hasChanged = true;
}

void SoundColor::renderColor(ColorChannel& prevColor, Spectrum& spectrum) {

	double r = 0., g = 0., b = 0.;
	size_t binCount = spectrum.getBinCount();

	double curAvg = spectrum.getAverageEnergyDB() + settings.noiseFloor;

	if(curAvg < 0)
		curAvg = 0;

	//Loosely track the average DB
	if(prevColor.avg > curAvg) {
		//Slope limiting
		prevColor.avg -= std::min(settings.slopeLimitAvg, prevColor.avg - curAvg);
	}
	else {
		//Jump up to current average
		prevColor.avg = curAvg;
	}

	//Scale to be applied to each bin
	double scale = 1. / (settings.dbScaler * binCount);

	for(unsigned int i = 0; i < binCount; ++i) {
		FrequencyBin& bin = spectrum.getByIndex(i);
		double f = bin.getFreqCenter();

		if(f > settings.fEnd)
			break;

		if(f >= settings.fStart) {
			Color c = frequencyColors[i];
			double db = bin.getEnergyDB();

			//Bass boost
			if(f <= settings.bassFreq)
				db += settings.bassBoost;

			//Trebble boost
			if(f >= settings.trebbleFreq)
				db += settings.trebbleBoost;

			//Raise by noise floor, subtract loosly-tracking average
			db += settings.noiseFloor - prevColor.avg;

			//Reject anything below the noise floor
			if(db <= 0)
				continue;

			//Scale partially based on average level
			db *= (settings.dbFactor + settings.avgFactor*prevColor.avg);

			//Add weighted color to running color average
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

	std::cout << r << ' ' << g << ' ' << b << std::endl;

	Color c(r, g, b);
	double h = c.getHue(), s = c.getHSVSaturation(), v = c.getValue();

	//Enforce saturation minimum
	c = Color::HSV(h, std::max(settings.minSaturation, s), v);

	//Filter the color
	prevColor.c.filter(c, settings.filterStrength);
}
