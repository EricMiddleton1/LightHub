#include "LightEffectSoundSolid.hpp"


LightEffectSoundSolid::LightEffectSoundSolid(
	std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer,
		LightEffectSoundSolid::Channel _channel)
	:	LightEffect({LightStrip::Type::Analog, LightStrip::Type::Digital},
		{{"bass freq", 150.}, {"trebble freq", 4000.}, {"bass boost", 10.},
		{"trebble boost", 0.}, {"start frequency", 0.}, {"end frequency", 20000.},
		{"db scaler", 150.}, {"db factor", 1.}, {"average factor", 0.5},
		{"noise floor", 50.}, {"average filter strength", 0.4}, {"min saturation", 0.7},
		{"color filter strength", 0.8}, {"threshold", 0.}})
	,	spectrumAnalyzer{_spectrumAnalyzer}
	,	channel{_channel} {
	
}

void LightEffectSoundSolid::tick() {

	switch(channel) {
		case LightEffectSoundSolid::Channel::Left:
			renderColor(spectrumAnalyzer->getLeftSpectrum());
		break;

		case LightEffectSoundSolid::Channel::Center:
			renderColor(spectrumAnalyzer->getCenterSpectrum());
		break;

		case LightEffectSoundSolid::Channel::Right:
			renderColor(spectrumAnalyzer->getRightSpectrum());
		break;
	}
}

void LightEffectSoundSolid::updateStrip(std::shared_ptr<LightStrip> strip) {
	auto buffer = strip->getBuffer();

	buffer->setAll(c);
}

void LightEffectSoundSolid::renderColor(Spectrum spectrum) {
	static int bassIndex = -1, endIndex = -1;
	static double avg = 0.;

	double bassFreq = getParameter("bass freq").getValue().getNumber();
	double trebbleFreq = getParameter("trebble freq").getValue().getNumber();
	double bassBoost = getParameter("bass boost").getValue().getNumber();
	double trebbleBoost = getParameter("trebble boost").getValue().getNumber();
	double fStart = getParameter("start frequency").getValue().getNumber();
	double fEnd = getParameter("end frequency").getValue().getNumber();
	double dbScaler = getParameter("db scaler").getValue().getNumber();
	double dbFactor = getParameter("db factor").getValue().getNumber();
	double avgFactor = getParameter("average factor").getValue().getNumber();
	double noiseFloor = getParameter("noise floor").getValue().getNumber();
	double avgFilterStrength = getParameter("average filter strength").getValue().getNumber();
	double minSaturation = getParameter("min saturation").getValue().getNumber();
	double filterStrength = getParameter("color filter strength").getValue().getNumber();
	double threshold = getParameter("threshold").getValue().getNumber();

	double r = 0., g = 0., b = 0.;
	size_t binCount = spectrum.getBinCount();

	if(bassIndex == -1) {
		for(bassIndex = 0; (bassIndex < binCount) &&
			(spectrum.getByIndex(bassIndex).getFreqCenter() <= bassFreq); ++bassIndex);
		std::cout << "Bass Index: " << bassIndex << std::endl;
	}
	if(endIndex == -1) {
		for(endIndex = 0; (endIndex < binCount) &&
			(spectrum.getByIndex(endIndex).getFreqCenter() <= fEnd); ++endIndex);
		std::cout << "End Index: " << endIndex << std::endl;
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
			float hue = (f <= bassFreq) ? 40.f*std::pow((double)i/(bassIndex-1), 4.)
				: (45.f + 240.f * (i-bassIndex) / (binCount - bassIndex - 1));

			Color c = Color::HSV(hue, 1.f, 1.f);
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
			if(db < 0)
				continue;

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
	if(largest > 255) {
		double scale = 255. / largest;

		r *= scale;
		g *= scale;
		b *= scale;
	}

	Color cTmp(r, g, b);
	double h = cTmp.getHue(), s = cTmp.getHSVSaturation(), v = cTmp.getValue();

	if(v < threshold) {
		v = 0.;
	}

	//Enforce saturation minimum
	cTmp = Color::HSV(h, std::max(minSaturation, s), v);

	//Filter the color
	c.filter(cTmp, filterStrength);
}
