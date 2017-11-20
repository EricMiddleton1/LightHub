#include "LightEffectStripBeat.hpp"

#include <cmath>
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>

LightEffectStripBeat::LightEffectStripBeat(
	std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer)
	:	LightEffect{{{"reverse", true}}}
	,	spectrumAnalyzer(_spectrumAnalyzer)
	,	smoothed(spectrumAnalyzer->getLeftSpectrum().getBinCount())
	,	delta(smoothed.size())
	,	avgEnergy{1.f} {
	
	for(int i = 0; i < AREA_COUNT; ++i) {
		areas.push_back(i);
	}

	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::shuffle(areas.begin(), areas.end(), std::default_random_engine(seed));
}

bool LightEffectStripBeat::validateLight(const std::shared_ptr<Light>& light) {
	return light->getSize() > 1;
}

void LightEffectStripBeat::tick() {
	const double NOISE_FLOOR = 50.;

	Spectrum spec = spectrumAnalyzer->getMonoSpectrum();
	size_t binCount = spec.getBinCount();

	double threshold = (AREA_COUNT-areas.size())*(MAX_THRESHOLD-MIN_THRESHOLD)
		/(AREA_COUNT-1) + MIN_THRESHOLD;

	for(size_t i = 0; i < binCount; ++i) {
		auto bin = spec.getByIndex(i);
		double db = 0.;

		db = bin.getEnergyDB() + NOISE_FLOOR;
		if(db < 0.) {
			db = 0.;
		}

		//Convert to range [0,1]
		double top = 1.4*(db / NOISE_FLOOR);

		delta[i] = top - smoothed[i];

		if(delta[i] > threshold) {
			bool added = false;
			for(auto& beat : beats) {
				if(beat.willAccept(i)) {
					added = true;
					beat.add(i);
				}
			}

			if(!added) {
				if(areas.empty()) {
					areas.push_back(beats[0].area);
					beats.erase(beats.begin());
				}

				beats.emplace_back(i, areas[0]);
				areas.erase(areas.begin());
			}
		}

		//Smooth with exponential filter
		smoothed[i] = top*0.5 + smoothed[i]*0.5;
	}

	for(int i = 0; i < beats.size(); ++i) {
		if(!beats[i].update(smoothed)) {
			areas.push_back(beats[i].area);
			beats.erase(beats.begin()+i);
			--i;
		}
	}

	//Smooth with exponential filter
	avgEnergy = 0.25*spec.getAverageEnergy() + 0.75*avgEnergy;
}

void LightEffectStripBeat::updateLight(std::shared_ptr<Light>& light) {
	bool reverse = getParameter("reverse").getValue().getBool();
	
	auto buffer = light->getBuffer();
	buffer.setAll({});

	size_t ledCount = buffer.getSize(),
		binCount = smoothed.size();
	
	for(auto& beat : beats) {
		int start = beat.bins[0], end = beat.bins[beat.bins.size()-1];

		float h = 255.f*(start+end)/2*240./(binCount-1)/360.f, s = 255.f,
			v = 255.f*beat.value;

		if(v < 0) {
			v = 0;
		}
		else if(v > 255) {
			v = 255;
		}

		Color c = Color::HSV(h, s, v);

		size_t ledStart = beat.area*ledCount/AREA_COUNT,
			ledEnd = (beat.area+1)*ledCount/AREA_COUNT - 1;
		for(size_t j = ledStart; j < ledEnd; ++j) {
			unsigned int x = reverse ? (ledCount - j - 1) : j;
			buffer[x] = c;
		}
	}
}

LightEffectStripBeat::Beat::Beat(int bin, int _area)
	:	bins{{bin}}
	,	aliveTime{0}
	,	addTime{0}
	,	value{0.}
	,	valueMultiplier{1.}
	,	area{_area} {
}

bool LightEffectStripBeat::Beat::willAccept(int other) {
	int range = (aliveTime >= MERGE_TIMEOUT) ? 0 : 2;

	for(const auto& bin : bins) {
		if(std::abs(bin - other) <= range)
			return true;
	}

	return false;
}

void LightEffectStripBeat::Beat::add(int bin) {
	bins.push_back(bin);
	std::sort(bins.begin(), bins.end());

	addTime = aliveTime;
	valueMultiplier = 1.;
}

bool LightEffectStripBeat::Beat::update(const std::vector<double>& spectrum) {
	double sum = 0., max = 0.;

	aliveTime++;
	if(aliveTime >= TIMEOUT || ((aliveTime-addTime) >= ACTIVITY_TIMEOUT)) {
		return false;
	}

	for(const auto& bin : bins) {
		sum += spectrum[bin];
	}

	value = std::min(1., sum / bins.size());

	valueMultiplier *= VALUE_FACTOR;
	value *= valueMultiplier;

	if(value < MIN_VALUE) {
		return false;
	}

	return true;
}
