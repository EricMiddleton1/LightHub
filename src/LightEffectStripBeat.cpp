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
			int bestMatch = -1, bestMatchIndex;
			for(int j = 0; j < bars.size(); ++j) {
				int match = bars[j].willAccept(i);
				if(match > bestMatch) {
					bestMatch = match;
					bestMatchIndex = j;
				}
			}
			
			if(bestMatch > -1) {
				bars[bestMatchIndex].add(i);
			}
			else {
				if(areas.empty()) {
					areas.push_back(bars[0].area);
					bars.erase(bars.begin());
				}

				bars.emplace_back(i, areas[0]);
				areas.erase(areas.begin());
			}
		}

		//Smooth with exponential filter
		smoothed[i] = top*0.5 + smoothed[i]*0.5;
	}

	for(int i = 0; i < bars.size(); ++i) {
		if(!bars[i].update(smoothed)) {
			areas.push_back(bars[i].area);
			bars.erase(bars.begin()+i);
			--i;
		}
	}

	//Smooth with exponential filter
	avgEnergy = 0.25*spec.getAverageEnergy() + 0.75*avgEnergy;
}

void LightEffectStripBeat::updateLight(std::shared_ptr<Light>& light) {
	bool reverse = getParameter("reverse").getValue().getBool();
	
	auto buffer = light->getBuffer();
	buffer->setAll({});

	size_t ledCount = buffer->getSize(),
		binCount = smoothed.size();
	
	for(auto& bar : bars) {
		int start = bar.beats[0].bin, end = bar.beats[bar.beats.size()-1].bin;

		float h = 255.f*(start+end)/2*240./(binCount-1)/360.f, s = 255.f,
			v = 255.f*bar.value;

		if(v < 0) {
			v = 0;
		}
		else if(v > 255) {
			v = 255;
		}

		Color c = Color::HSV(h, s, v);

		size_t ledStart = bar.area*ledCount/AREA_COUNT,
			ledEnd = (bar.area+1)*ledCount/AREA_COUNT - 1;
		for(size_t j = ledStart; j < ledEnd; ++j) {
			unsigned int x = reverse ? (ledCount - j - 1) : j;
			buffer->at(x) = c;
		}
	}
}

LightEffectStripBeat::Bar::Bar(int bin, int _area)
	:	beats{{{bin, 0}}}
	,	aliveTime{0}
	,	addTime{0}
	,	value{0.}
	,	valueMultiplier{1.}
	,	area{_area} {
}

int LightEffectStripBeat::Bar::willAccept(int other) const {
	int bestMatch = MERGE_RANGE + 1;

	for(const auto& beat : beats) {
		int range = ((aliveTime - beat.startTime) >= MERGE_TIMEOUT) ? 0 : MERGE_RANGE;
		int dist = std::abs(beat.bin - other);

		if((dist <= range) && (dist < bestMatch)) {
			bestMatch = dist;
		}
	}

	return MERGE_RANGE - bestMatch;
}

void LightEffectStripBeat::Bar::add(int bin) {
	auto found = std::find_if(beats.begin(), beats.end(), [&bin](const auto& beat) {
		return bin == beat.bin;
	});

	if(found != beats.end()) {
		found->startTime = aliveTime;
	}
	else {
		beats.push_back({bin, aliveTime});
		std::sort(beats.begin(), beats.end(), [](const auto& b1, const auto& b2) {
			return b1.bin <= b2.bin;
		});
	}

	addTime = aliveTime;
	valueMultiplier = 1.;
}

bool LightEffectStripBeat::Bar::update(const std::vector<double>& spectrum) {
	double sum = 0., max = 0.;

	aliveTime++;
	if(aliveTime >= TIMEOUT || ((aliveTime-addTime) >= ACTIVITY_TIMEOUT)) {
		return false;
	}

	for(int i = 0; i < beats.size(); ++i) {
		if((aliveTime - beats[i].startTime) > ACTIVITY_TIMEOUT) {
			beats.erase(beats.begin() + i);
			--i;
		}
	}

	if(beats.empty()) {
		return false;
	}

	for(const auto& beat : beats) {
		sum += spectrum[beat.bin];
	}

	value = std::min(1., sum / beats.size());

	valueMultiplier *= VALUE_FACTOR;
	value *= valueMultiplier;

	if(value < MIN_VALUE) {
		return false;
	}

	return true;
}
