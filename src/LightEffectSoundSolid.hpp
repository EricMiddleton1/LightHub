#pragma once

#include <memory>

#include "ILightEffect.hpp"
#include "Color.hpp"
#include "SoundColor.hpp"

class LightEffectSoundSolid : public ILightEffect
{
public:
	LightEffectSoundSolid(std::shared_ptr<SpectrumAnalyzer>,
		const SoundColorSettings&);

private:
	virtual void tick() override;
	virtual void updateStrip(std::shared_ptr<LightStrip>) override;

	SoundColor soundColor;
	Color left, center, right;
};
