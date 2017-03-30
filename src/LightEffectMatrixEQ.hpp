#pragma once

#include <map>
#include <vector>
#include "LightEffect.hpp"
#include "Color.hpp"
#include "SpectrumAnalyzer.hpp"

class LightEffectMatrixEQ : public LightEffect
{
public:
	LightEffectMatrixEQ(std::shared_ptr<SpectrumAnalyzer>);
	~LightEffectMatrixEQ();

private:
	virtual void tick() override;
	virtual void updateStrip(std::shared_ptr<LightStrip>) override;

	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
	std::vector<double> heights;
};
