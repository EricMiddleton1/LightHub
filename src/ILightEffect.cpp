#include "ILightEffect.hpp"


ILightEffect::ILightEffect(const std::vector<LightStrip::Type>& _types)
	:	supportedTypes(_types) {
}

ILightEffect::~ILightEffect() {
}

void ILightEffect::addStrip(const std::shared_ptr<LightStrip>& strip) {
	std::unique_lock<std::mutex> stripsLock(stripsMutex);
	
	auto id = strip->getID();

	if(std::find_if(strips.begin(), strips.end(),
		[id](const auto& pair) {
			return pair.first == id;
		}) != strips.end()) {
		throw Exception(EXCEPTION_LIGHT_EFFECT_STRIP_ALREADY_CONNECTED,
			"ILightEffect::addStrip: Strip already connected");
	}

	if(find(supportedTypes.begin(), supportedTypes.end(), strip->getType())
		== supportedTypes.end()) {
		throw Exception(EXCEPTION_LIGHT_EFFECT_UNSUPPORTED_TYPE,
			"ILightEffect::addStrip: Strip type unsupported");
	}
	
	auto pair = std::pair<size_t, std::weak_ptr<LightStrip>>(id, strip);

	strips.push_back(pair);

	if(onAdd) {
		onAdd(pair);
	}
}

void ILightEffect::removeStrip(size_t id) {
	std::unique_lock<std::mutex> stripsLock(stripsMutex);

	//Find the node in the vector
	auto found = std::find_if(strips.begin(), strips.end(),
		[id](const auto& pair) {
			return pair.first == id;
		});

	if(found == strips.end()) {
		//We didn't find the node
		throw Exception(EXCEPTION_LIGHT_EFFECT_STRIP_NOT_FOUND,
			"ILightEffect::removeStrip: strip not found in vector");
	}

	if(onRemove) {
		onRemove(*found);
	}

	//Remove the node
	strips.erase(found);
}

void ILightEffect::update() {
	std::vector<size_t> deadStrips;

	//Call 'tick' (to allow effect to perform updates)
	tick();

	{
		std::unique_lock<std::mutex> stripsLock(stripsMutex);

		for(auto& pair : strips) {
			auto sharedStrip = pair.second.lock();

			if(sharedStrip) {
				updateStrip(sharedStrip);
			}
			else {
				deadStrips.push_back(pair.first);
			}
		}
	}

	for(auto &strip : deadStrips) {
		removeStrip(strip);
	}
}
