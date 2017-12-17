#pragma once

#include <map>
#include <vector>
#include "LightEffect.hpp"
#include "Color.hpp"
#include "SpectrumAnalyzer.hpp"

class LightEffectMatrixCircSpec : public LightEffect
{
public:
	LightEffectMatrixCircSpec(std::shared_ptr<SpectrumAnalyzer>);
	~LightEffectMatrixCircSpec();

private:
	static const int HISTORY_LENGTH = 22;
	virtual bool validateLight(const std::shared_ptr<Light>&) override;
	virtual void tick() override;
	virtual void updateLight(std::shared_ptr<Light>&) override;

	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
	std::vector<double> smoothed;
	std::vector<std::vector<double>> beats;
};
