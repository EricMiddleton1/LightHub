#include "LightEffect.hpp"

#include <stdexcept>
#include <algorithm>

#include "Matrix.hpp"

using namespace std;

LightEffect::LightEffect(const std::vector<Parameter>& _parameters, bool _requireMatrix)
	:	ConfigurableObject(_parameters)
	,	requireMatrix{_requireMatrix} {
}

LightEffect::~LightEffect() {
}

bool LightEffect::addLight(std::shared_ptr<Light>& light) {
	if(requireMatrix && !Matrix::isMatrix(light)) {
		return false;
	}
	
	std::unique_lock<std::mutex> lightLock(lightMutex);

	if(validateLight(light)) {
		auto fullName = light->getFullName();

		if(lights.find(fullName) != lights.end()) {
			cerr << "[Error] LightEffect::addLight: Light '" << fullName << "' already added"
				<< endl;

			return false;
		}
		else {
			lights.emplace(fullName, light);
		}
		
		if(onAdd) {
			onAdd(light);
		}

		return true;
	}
	else {
		return false;
	}
}

void LightEffect::removeLight(const std::string& fullName) {
	unique_lock<mutex> lightLock(lightMutex);

	if(lights.find(fullName) == lights.end()) {
		throw runtime_error(string("LightEffect::addLight: Light '") + fullName + "' not found");
	}
	else {
		auto sharedLight = lights[fullName].lock();
		if(sharedLight && onRemove) {
			onRemove(sharedLight);
		}
		lights.erase(fullName);
	}
}

void LightEffect::update() {
	vector<string> deadLights;

	//Call 'tick' (to allow effect to perform updates)
	tick();

	{
		unique_lock<mutex> lightLock(lightMutex);

		for(auto& light : lights) {
			auto sharedLight = light.second.lock();

			if(sharedLight) {
				updateLight(sharedLight);
			}
			else {
				deadLights.push_back(light.first);
			}
		}
	}

	for(auto &light : deadLights) {
		removeLight(light);
	}
}
