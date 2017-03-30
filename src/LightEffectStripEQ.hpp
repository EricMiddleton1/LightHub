#pragma once

#include <map>
#include <vector>
#include "LightEffect.hpp"
#include "Color.hpp"
#include "SpectrumAnalyzer.hpp"

class LightEffectStripEQ : public LightEffect
{
public:
	LightEffectStripEQ(std::shared_ptr<SpectrumAnalyzer>);

private:
	virtual void tick() override;
	virtual void updateStrip(std::shared_ptr<LightStrip> strip) override;

	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
	std::vector<double> smoothed;
	double avgEnergy;
};
