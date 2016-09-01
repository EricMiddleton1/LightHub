#pragma once

#include "ILightEffect.hpp"
#include "Color.hpp"
#include "SoundColor.hpp"

class LightEffectSoundSolid : public ILightEffect
{
public:
	LightEffectSoundSolid(std::shared_ptr<SpectrumAnalyzer>,
		const SoundColorSettings&);
	~LightEffectSoundSolid();

private:
	virtual void update() override;

	SoundColor soundColor;
};
