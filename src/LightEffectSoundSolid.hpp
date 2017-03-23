#pragma once

#include <memory>

#include "ILightEffect.hpp"
#include "Color.hpp"
#include "SoundColor.hpp"

class LightEffectSoundSolid : public ILightEffect
{
public:
	enum class Channel {
		Left = 0,
		Center,
		Right
	};

	LightEffectSoundSolid(std::shared_ptr<SpectrumAnalyzer>,
		const SoundColorSettings&, Channel channel = Channel::Center);

private:
	virtual void tick() override;
	virtual void updateStrip(std::shared_ptr<LightStrip>) override;
	
	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
	SoundColor soundColor;
	Channel channel;

	Color c;
};
