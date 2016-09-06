#pragma once

#include "SpectrumAnalyzer.hpp"


struct BeatDetectorSettings
{
	double freqBassMin;
	double freqBassMax;
	
	double freqTrebbleMin;
	double freqTrebbleMax;

	double noiseFloorBass;
	double noiseFloorTrebble;

	double thresholdBass;
	double thresholdTrebble;

	double filter;
};


class BeatDetector
{
public:
	BeatDetector(std::shared_ptr<SpectrumAnalyzer>, const BeatDetectorSettings&);
	~BeatDetector();

	bool getBeat(bool*, bool*, bool*, bool*);

private:
	void cbSpectrum(SpectrumAnalyzer*, std::shared_ptr<Spectrum> left,
		std::shared_ptr<Spectrum> right);

	void beatDetect(Spectrum& spectrum, double* bassAvg, double* trebbleAvg,
		bool* bassBeat, bool* trebbleBeat);

	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
	BeatDetectorSettings settings;

	bool bassBeatLeft, bassBeatRight, trebbleBeatLeft, trebbleBeatRight;
	bool changed;

	double bassAvgLeft, bassAvgRight, trebbleAvgLeft, trebbleAvgRight;

};
