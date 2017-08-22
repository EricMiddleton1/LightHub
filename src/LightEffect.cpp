#include "LightEffect.hpp"

#include <stdexcept>
#include <algorithm>

LightEffect::LightEffect(const std::vector<LightStrip::Type>& _types,
	const std::vector<Parameter>& _parameters)
	:	ConfigurableObject(_parameters)
	,	supportedTypes(_types) {
}

LightEffect::~LightEffect() {
}

void LightEffect::addStrip(const std::shared_ptr<LightStrip>& strip) {
	std::unique_lock<std::mutex> stripsLock(stripsMutex);
	
	auto id = strip->getID();

	if(std::find_if(strips.begin(), strips.end(),
		[id](const auto& pair) {
			return pair.first == id;
		}) != strips.end()) {
		throw Exception(EXCEPTION_LIGHT_EFFECT_STRIP_ALREADY_CONNECTED,
			"LightEffect::addStrip: Strip already connected");
	}

	if(find(supportedTypes.begin(), supportedTypes.end(), strip->getType())
		== supportedTypes.end()) {
		throw Exception(EXCEPTION_LIGHT_EFFECT_UNSUPPORTED_TYPE,
			"LightEffect::addStrip: Strip type unsupported");
	}
	
	auto pair = std::pair<size_t, std::weak_ptr<LightStrip>>(id, strip);

	strips.push_back(pair);

	if(onAdd) {
		onAdd(pair);
	}

	if( (strips.size() == 1) && onStart ) {
		onStart();
	}
}

void LightEffect::removeStrip(size_t id) {
	std::unique_lock<std::mutex> stripsLock(stripsMutex);

	//Find the node in the vector
	auto found = std::find_if(strips.begin(), strips.end(),
		[id](const auto& pair) {
			return pair.first == id;
		});

	if(found == strips.end()) {
		//We didn't find the node
		throw Exception(EXCEPTION_LIGHT_EFFECT_STRIP_NOT_FOUND,
			"LightEffect::removeStrip: strip not found in vector");
	}

	if(onRemove) {
		onRemove(*found);
	}

	//Remove the node
	strips.erase(found);

	if( strips.empty() && onStop ) {
		onStop();
	}
}

void LightEffect::update() {
	std::vector<size_t> deadStrips;

	{
		std::unique_lock<std::mutex> stripsLock(stripsMutex);
		
		if(!strips.empty()) {
			tick();

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
	}

	for(auto &strip : deadStrips) {
		removeStrip(strip);
	}
}
