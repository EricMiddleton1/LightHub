#pragma once

#include <map>
#include <vector>
#include "ILightEffect.hpp"
#include "Color.hpp"
#include "SoundColor.hpp"

class LightEffectMatrixEQ : public ILightEffect
{
public:
	LightEffectMatrixEQ(std::shared_ptr<SpectrumAnalyzer>, unsigned int bandCount);
	~LightEffectMatrixEQ();

private:
	virtual void tick() override;
	virtual void updateStrip(std::shared_ptr<LightStrip>) override;

	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
	unsigned int bandCount;
	std::vector<double> heights;
};
