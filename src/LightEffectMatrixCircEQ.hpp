#pragma once

#include <map>
#include <vector>
#include "LightEffect.hpp"
#include "Color.hpp"
#include "SpectrumAnalyzer.hpp"

class LightEffectMatrixCircEQ : public LightEffect
{
public:
	LightEffectMatrixCircEQ(std::shared_ptr<SpectrumAnalyzer>);
	~LightEffectMatrixCircEQ();

private:
	virtual bool validateLight(const std::shared_ptr<Light>&) override;
	virtual void tick() override;
	virtual void updateLight(std::shared_ptr<Light>&) override;

	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
	std::vector<double> heights;
};
