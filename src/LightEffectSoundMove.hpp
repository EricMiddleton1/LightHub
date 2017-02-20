#pragma once

#include <memory>

#include "ILightEffect.hpp"
#include "Color.hpp"
#include "SpectrumAnalyzer.hpp"

class LightEffectSoundMove : public ILightEffect
{
public:
	LightEffectSoundMove(std::shared_ptr<SpectrumAnalyzer>);
	~LightEffectSoundMove();

private:
	virtual void update() override;

	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
};
