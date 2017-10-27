#pragma once

#include <memory>
#include <vector>

#include "LightEffect.hpp"
#include "Color.hpp"
#include "SpectrumAnalyzer.hpp"

class LightEffectSoundSolid : public LightEffect
{
public:
	enum class Channel {
		Left = 0,
		Center,
		Right
	};

	LightEffectSoundSolid(std::shared_ptr<SpectrumAnalyzer>,
		Channel channel = Channel::Center);


private:
	virtual bool validateLight(const std::shared_ptr<Light>&);
	virtual void tick() override;
	virtual void updateLight(std::shared_ptr<Light>&) override;

	void renderColor(Spectrum spectrum);
	
	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
	Channel channel;

	Spectrum prevSpectrum;
	int bassIndex, endIndex;
	double avg;


	Color c;
};
