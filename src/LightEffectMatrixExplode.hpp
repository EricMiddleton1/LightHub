#pragma once

#include <map>
#include <vector>
#include "LightEffect.hpp"
#include "Color.hpp"
#include "SpectrumAnalyzer.hpp"

class LightEffectMatrixExplode : public LightEffect
{
public:
	LightEffectMatrixExplode(std::shared_ptr<SpectrumAnalyzer>);
	~LightEffectMatrixExplode();

private:
	virtual bool validateLight(const std::shared_ptr<Light>&) override;
	virtual void tick() override;
	virtual void updateLight(std::shared_ptr<Light>&) override;

	Color renderColor(const Color& prev, Spectrum spectrum);

	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
	std::vector<Color> colors;
};
