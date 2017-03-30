#pragma once

#include <memory>

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
	virtual void tick() override;
	virtual void updateStrip(std::shared_ptr<LightStrip>) override;

	void renderColor(Spectrum spectrum);
	
	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
	Channel channel;

	Color c;
};
