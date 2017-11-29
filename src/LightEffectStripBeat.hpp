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

	struct Bar {
		struct Beat {
			int bin;
			size_t startTime;
		};

		static const int MERGE_RANGE = 2;
		static const int MERGE_TIMEOUT = 10;
		static const int ACTIVITY_TIMEOUT = 50;
		static const int TIMEOUT = 100000;
		static constexpr double MIN_VALUE = 0.;
		static constexpr double VALUE_FACTOR = 0.9;

		Bar(int bin, int area);
		Bar(const Beat&);

		int willAccept(int bin) const;
		void add(int bin);

		bool update(const std::vector<double>& spectrum);

		std::vector<Beat> cull();

		std::vector<Beat> beats;
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
	std::vector<Bar> bars;
	std::vector<int> areas;
	double avgEnergy;
};
