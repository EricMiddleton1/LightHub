#pragma once

#include "ILightEffect.hpp"
#include "Color.hpp"
#include "SoundColor.hpp"

class LightEffectSoundMove : public ILightEffect
{
public:
	LightEffectSoundMove(std::shared_ptr<SpectrumAnalyzer>,
		const SoundColorSettings&);
	~LightEffectSoundMove();

private:
	virtual void update() override;

	SoundColor soundColor;
};
