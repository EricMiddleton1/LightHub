#pragma once

#include <map>
#include <vector>
#include "ILightEffect.hpp"
#include "Color.hpp"
#include "SoundColor.hpp"

class LightEffectStripStEQ : public ILightEffect
{
public:
	LightEffectStripStEQ(std::shared_ptr<SpectrumAnalyzer>, size_t maxBins);
	~LightEffectStripStEQ();

	virtual void addNode(const std::shared_ptr<LightNode>&) override;
	virtual void removeNode(std::shared_ptr<LightNode>) override;

private:
	virtual void update() override;

	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
	std::map<std::shared_ptr<LightNode>, std::vector<double>> leftValues, rightValues;
	size_t maxBins;
	double leftAvgEnergy, rightAvgEnergy;
};
