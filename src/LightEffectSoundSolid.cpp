#include "LightEffectSoundSolid.hpp"


LightEffectSoundSolid::LightEffectSoundSolid(
	std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer,
		LightEffectSoundSolid::Channel _channel)
	:	LightEffect{{{"bass freq", 150.}, {"trebble freq", 4000.}, {"bass boost", 10.},
		{"trebble boost", 0.}, {"start frequency", 0.}, {"end frequency", 20000.},
		{"db scaler", 150.}, {"db factor", 1.}, {"average factor", 0.5},
		{"change factor", 0.},
		{"noise floor", 50.}, {"average filter strength", 0.4}, {"min saturation", 0.7},
		{"color filter strength", 0.8}, {"threshold", 0.}}}
	,	spectrumAnalyzer{_spectrumAnalyzer}
	,	channel{_channel}
	,	prevSpectrum{0, 0, 0}
	,	bassIndex{-1}
	,	endIndex{-1}
	,	avg{0.} {
}

bool LightEffectSoundSolid::validateLight(const std::shared_ptr<Light>& light) {
	return true;
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

void LightEffectSoundSolid::updateLight(std::shared_ptr<Light>& light) {
	auto buffer = light->getBuffer();
	
	buffer->setAll(c);
	buffer->setTransitionPeriod({5, 10, 10});
}

void LightEffectSoundSolid::renderColor(Spectrum spectrum) {
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

	double r = 0., g = 0., b = 0.;
	size_t binCount = spectrum.getBinCount();

	if(bassIndex == -1) {
		for(bassIndex = 0; (bassIndex < binCount) &&
			(spectrum.getByIndex(bassIndex).getFreqCenter() <= bassFreq); ++bassIndex);
		std::cout << "Bass Index: " << bassIndex << std::endl;
		
		for(endIndex = 0; (endIndex < binCount) &&
			(spectrum.getByIndex(endIndex).getFreqCenter() <= fEnd); ++endIndex);
		std::cout << "End Index: " << endIndex << std::endl;

		prevSpectrum = spectrum;
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

			Color c = Color::HSV(255.f*hue/360.f, 255, 255);

			double db = bin.getEnergyDB();
			
			FrequencyBin& prevBin = prevSpectrum.getByIndex(i);
			double change = db - prevBin.getEnergyDB();
			if(change < 0)
				change = 0;

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
			db += avgFactor*avg + changeFactor*change;

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
	
	uint8_t h = cTmp.getHue(), s = cTmp.getSat(), v = cTmp.getVal();

	if(v < threshold) {
		v = 0;
	}

	//Enforce saturation minimum
	cTmp = Color::HSV(h, std::max(minSaturation, s), v);

	//Filter the color
	c.filter(cTmp, filterStrength);

	//Update previous spectrum
	prevSpectrum = spectrum;
}
