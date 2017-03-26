#pragma once

#include <map>
#include <vector>
#include "ILightEffect.hpp"
#include "Color.hpp"
#include "SoundColor.hpp"

class LightEffectStripEQ : public ILightEffect
{
public:
	LightEffectStripEQ(std::shared_ptr<SpectrumAnalyzer>, bool reverse = false);

private:
	virtual void tick() override;
	virtual void updateStrip(std::shared_ptr<LightStrip> strip) override;

	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
	std::vector<double> smoothed;
	double avgEnergy;
	bool reverse;
};
