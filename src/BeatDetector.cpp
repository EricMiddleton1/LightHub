#include "BeatDetector.hpp"

#include <cstdio> //printf

BeatDetector::BeatDetector(std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer,
	const BeatDetectorSettings& _settings)
	:	spectrumAnalyzer(_spectrumAnalyzer)
	,	settings(_settings)
	,	bassBeatLeft{false}
	,	bassBeatRight{false}
	,	trebbleBeatLeft{false}
	,	trebbleBeatRight{false}
	,	changed{false}
	,	bassAvgLeft{0}
	,	bassAvgRight{0}
	,	trebbleAvgLeft{0}
	,	trebbleAvgRight{0} {

	//Convert noise floor from DB to absolute values
	settings.noiseFloorBass = std::pow(10., settings.noiseFloorBass/20.);
	settings.noiseFloorTrebble = std::pow(10., settings.noiseFloorTrebble/20.);

	//Register the callback with the spectrum analyzer
	spectrumAnalyzer->addListener([this](SpectrumAnalyzer* sa,
		std::shared_ptr<Spectrum> left, std::shared_ptr<Spectrum> right) {
		cbSpectrum(sa, left, right);
	});
}

BeatDetector::~BeatDetector() {
	//TODO: Remove callback from spectrum analyzer
}

bool BeatDetector::getBeat(bool* leftBass, bool* leftTrebble, bool* rightBass, bool* rightTrebble) {
	if(leftBass != nullptr)
		*leftBass = bassBeatLeft;
	
	if(leftTrebble != nullptr)
		*leftTrebble = trebbleBeatLeft;
	
	if(rightBass != nullptr)
		*rightBass = bassBeatRight;
	
	if(rightTrebble != nullptr)
		*rightTrebble = trebbleBeatRight;
	
	if(changed) {
		changed = false;

		return true;
	}
	else {
		return true;
	}
}

void BeatDetector::cbSpectrum(SpectrumAnalyzer*, std::shared_ptr<Spectrum> left,
	std::shared_ptr<Spectrum> right) {

	//Left beat detection
	beatDetect(*left.get(), &bassAvgLeft, &trebbleAvgLeft, &bassBeatLeft, &trebbleBeatLeft);

	//Right beat detection
	//beatDetect(*right.get(), &bassAvgRight, &trebbleAvgRight, &bassBeatRight, &trebbleBeatRight);

	//Indicate that the beat detection has changed
	changed = true;
}

void BeatDetector::beatDetect(Spectrum& spectrum, double* bassAvg, double* trebbleAvg,
	bool* bassBeat, bool* trebbleBeat) {

	//Calculate bass and trebble energy
	double bassEnergy = 0., trebbleEnergy = 0.;
	for(size_t i = 0; i < spectrum.getBinCount(); ++i) {
		auto& bin = spectrum.getByIndex(i);
		double f = bin.getFreqCenter();

		if(f >= settings.freqBassMin && f < settings.freqBassMax)
			bassEnergy += bin.getEnergy();
		else if(f >= settings.freqTrebbleMin && f < settings.freqTrebbleMax)
			trebbleEnergy += bin.getEnergy();
	}

	//Check for bass, trebble beats
	*bassBeat = bassEnergy > (*bassAvg)*settings.thresholdBass &&
		bassEnergy > settings.noiseFloorBass;
	
	*trebbleBeat = trebbleEnergy > (*trebbleAvg)*settings.thresholdTrebble &&
		trebbleEnergy > settings.noiseFloorTrebble;

	//Update averages
	*bassAvg = *bassAvg*settings.filter + bassEnergy*(1. - settings.filter);
	*trebbleAvg = *trebbleAvg*settings.filter + trebbleEnergy*(1. - settings.filter);

	printf("Avg:\t%1.5lf\t\tCurrent:\t%1.5lf\n", *trebbleAvg, trebbleEnergy);
}
