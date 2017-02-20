#pragma once

#include <map>
#include <vector>
#include "ILightEffect.hpp"
#include "Color.hpp"
#include "SoundColor.hpp"

class LightEffectMatrixEQ : public ILightEffect
{
public:
	LightEffectMatrixEQ(std::shared_ptr<SpectrumAnalyzer>);
	~LightEffectMatrixEQ();

	virtual void addNode(const std::shared_ptr<LightNode>&) override;
	virtual void removeNode(std::shared_ptr<LightNode>) override;

private:
	virtual void update() override;

	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
	std::map<std::shared_ptr<LightNode>, std::vector<double>> values;
};
