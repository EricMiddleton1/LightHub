#pragma once

#include <map>
#include <vector>
#include "LightEffect.hpp"
#include "Color.hpp"
#include "SpectrumAnalyzer.hpp"

class LightEffectStripBeat : public LightEffect
{
public:
	LightEffectStripBeat(std::shared_ptr<SpectrumAnalyzer>);

private:
	static const int AREA_COUNT = 8;
	static constexpr double MIN_THRESHOLD = 0.15;
	static constexpr double MAX_THRESHOLD = 0.6;

	struct Beat {
		static const int MERGE_TIMEOUT = 6;
		static const int ACTIVITY_TIMEOUT = 10;
		static const int TIMEOUT = 25;
		static constexpr double MIN_VALUE = 0.1;
		static constexpr double VALUE_FACTOR = 0.9;

		Beat(int bin, int area);

		bool willAccept(int bin);
		void add(int bin);

		bool update(const std::vector<double>& spectrum);

		std::vector<int> bins;
		size_t aliveTime, addTime;
		double value, valueMultiplier;
		int area;
	};

	virtual bool validateLight(const std::shared_ptr<Light>&) override;
	virtual void tick() override;
	virtual void updateLight(std::shared_ptr<Light>& light) override;

	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer;
	std::vector<double> smoothed;
	std::vector<double> delta;
	std::vector<Beat> beats;
	std::vector<int> areas;
	double avgEnergy;
};
