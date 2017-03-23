#pragma once

#include <memory>
#include <vector>
#include <string>
#include <mutex>

#include <boost/signals2.hpp>

#include "SpectrumAnalyzer.hpp"
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

	double avgFilterStrength; //Strength for bin average filter

	double minSaturation; //minimum saturation value for color in HSV colorspace

	double filterStrength;//Strength of exponential color filter (0 is no filter)

	double centerSpread; //[0, 1] amount of center that is spread to left/right

	enum {
		MONO,
		CENTER
	} centerBehavior;

	std::string toString();
};


class SoundColor
{
public:
	SoundColor(const SoundColorSettings&);

	Color getColor(Spectrum);

private:
	void renderColor(Spectrum&);

	SoundColorSettings settings;

	Color c;
	double avg;
};
