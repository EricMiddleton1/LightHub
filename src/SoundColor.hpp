#pragma once

#include <memory>
#include <vector>
#include <string>

#include <boost/signals2.hpp>

#include "SpectrumAnalyzer.hpp"
#include "BeatDetector.hpp"
#include "Color.hpp"

struct SoundColorSettings
{
	double bassFreq; //Frequency of bass end (anchored to color orange)
	double trebbleFreq; //Frequency of trebble start

	double bassBoost; //dB boost from fMin to freqBass
	double trebbleBoost; //dB boost from freqTrebble to fMax

	double fStart; //Frequency to start rendering color
	double fEnd; //Frequency to stop rendering color

	double dbScaler; //Scale factor for each bin

	//db is calculated by the following formula:
	//db *= (dbFactor + avgFactor*avg)
	double dbFactor;
	double avgFactor;

	double noiseFloor; //Noise floor in dB

	double slopeLimitAvg; //Slope limiter for average bin level

	double minSaturation; //minimum saturation value for color in HSV colorspace

	double filterStrength;//Strength of exponential color filter (0 is no filter)

	double centerSpread; //[0, 1] amount of center that is spread to left/right

	enum {
		MONO,
		CENTER
	} centerBehavior;

	BeatDetectorSettings beatDetectorSettings; //Settings for beat detector


	std::string toString();
};


class SoundColor
{
public:
	SoundColor(std::shared_ptr<SpectrumAnalyzer>,
		const SoundColorSettings&);
	~SoundColor();

	bool changed();

	void getColor(Color* left, Color* center, Color* right);

private:
	struct ColorChannel {
		Color c;
		double avg;
	};
	void cbSpectrum(SpectrumAnalyzer*, std::shared_ptr<Spectrum> left,
		std::shared_ptr<Spectrum> right);

	void renderColor(ColorChannel&, Spectrum&);

	SoundColorSettings settings;

	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;

	BeatDetector beatDetector;

	std::vector<Color> frequencyColors;

	ColorChannel left, center, right;

	bool hasChanged;
};
