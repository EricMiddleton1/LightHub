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
	virtual bool validateLight(const std::shared_ptr<Light>&) override;
	virtual void tick() override;
	virtual void updateLight(std::shared_ptr<Light>& light) override;

	void smoothSpectrum(std::vector<double>& out, double& avg, const Spectrum& spectrum);

	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
	std::vector<double> leftSmoothed, rightSmoothed, monoSmoothed;
	double leftAvg, rightAvg, monoAvg;
};
