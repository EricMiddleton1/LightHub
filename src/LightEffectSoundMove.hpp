#pragma once

#include <memory>

#include "ILightEffect.hpp"
#include "Color.hpp"
#include "SpectrumAnalyzer.hpp"

class LightEffectSoundMove : public ILightEffect
{
public:
	LightEffectSoundMove(std::shared_ptr<SpectrumAnalyzer>);

private:
	virtual void tick() override;
	virtual void updateStrip(std::shared_ptr<LightStrip>) override;

	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
	Color curColor;
};
