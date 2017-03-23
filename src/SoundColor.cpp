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


SoundColor::SoundColor(std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer,
	const SoundColorSettings& _settings)
	:	settings(_settings)
	,	spectrumAnalyzer{_spectrumAnalyzer} {

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
	
	//Determine the corresponding bin to freqTrebble
	size_t trebbleBin = 0;
	for(trebbleBin = bassBin; trebbleBin < spectrum->getBinCount(); ++trebbleBin) {
		auto& bin = spectrum->getByIndex(trebbleBin);

		if(bin.getFreqCenter() >= settings.trebbleFreq)
			break;
	}

	for(size_t i = 0; i < bassBin; i++) {
		double hue = i * 30. / (bassBin);

		Color c = Color::HSV(hue, 1., 1.);

		frequencyColors.push_back(c);
	}

	for(size_t i = bassBin; i < spectrum->getBinCount(); i++) {
		double hue = 30. + (i - bassBin) * 210. /
			(spectrum->getBinCount() - bassBin - 1);

		Color c = Color::HSV(hue, 1., 1.);

		frequencyColors.push_back(c);
	}

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

Color SoundColor::getLeftColor() const {
	std::unique_lock<std::mutex> colorLock(colorMutex);
	
	return left.c;
}

Color SoundColor::getCenterColor() const {
	std::unique_lock<std::mutex> colorLock(colorMutex);

	return center.c;
}

Color SoundColor::getRightColor() const {
	std::unique_lock<std::mutex> colorLock(colorMutex);

	return right.c;
}

void SoundColor::cbSpectrum(SpectrumAnalyzer*,
	std::shared_ptr<Spectrum> leftSpectrum,
	std::shared_ptr<Spectrum> rightSpectrum) {

	std::unique_lock<std::mutex> colorLock(colorMutex);

	renderColor(left, leftSpectrum);
	renderColor(right, rightSpectrum);
}

void SoundColor::renderColor(ColorChannel& prevColor,
	std::shared_ptr<Spectrum> spectrum) {

	double r = 0., g = 0., b = 0.;
	size_t binCount = spectrum->getBinCount();

	double curAvg = spectrum->getAverageEnergyDB() + settings.noiseFloor;

	if(curAvg < 0)
		curAvg = 0;

	double absFloor = std::pow(10., -100);
	double absBoost = std::pow(10., settings.bassBoost/20.);
	
	prevColor.avg = prevColor.avg*settings.avgFilterStrength
		+ curAvg*(1. - settings.avgFilterStrength);

	//Scale to be applied to each bin
	double scale = 1. / settings.dbScaler;

	for(unsigned int i = 0; i < binCount; ++i) {
		FrequencyBin& bin = spectrum->getByIndex(i);
		double f = bin.getFreqCenter();

		if(f > settings.fEnd)
			break;

		if(f >= settings.fStart) {
			Color c = frequencyColors[i];
			//Color c = Color::HSV(240.f * i / (binCount - 1), 1.f, 1.f);
			double db = bin.getEnergyDB();
			double energy = bin.getEnergy();

			//Bass boost
			if(f <= settings.bassFreq)
			db += settings.bassBoost;

			//Trebble boost
			if(f >= settings.trebbleFreq)
				db += settings.trebbleBoost;
			
			//Raise by noise floor, subtract loosly-tracking average
			db += settings.noiseFloor - prevColor.avg;

			//Reject anything below the average
			if(db < 0)
				continue;

			//Scale partially based on average level
			db *= settings.dbFactor;
			db += settings.avgFactor*prevColor.avg;

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

	Color c(r, g, b);
	double h = c.getHue(), s = c.getHSVSaturation(), v = c.getValue();

	//Enforce saturation minimum
	c = Color::HSV(h, std::max(settings.minSaturation, s), v);

	//Filter the color
	prevColor.c.filter(c, settings.filterStrength);
}
