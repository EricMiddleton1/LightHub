#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <utility>
#include <mutex>

#include "Light.hpp"
#include "Parameter.hpp"
#include "ConfigurableObject.hpp"

class Rhopalia;

class LightEffect : public ConfigurableObject
{
public:
	LightEffect(const std::vector<Parameter>& parameters, bool requireMatrix = false);
	virtual ~LightEffect();

	bool addLight(std::shared_ptr<Light>&);
	void removeLight(const std::string& name);

protected:
	virtual bool validateLight(const std::shared_ptr<Light>&) = 0;
	virtual void tick() = 0;
	virtual void updateLight(std::shared_ptr<Light>&) = 0;

	std::function<void(std::shared_ptr<Light>&)> onAdd, onRemove;

private:
	friend class Rhopalia;
	
	void update();

	std::unordered_map<std::string, std::weak_ptr<Light>> lights;
	mutable std::mutex lightMutex;

	bool requireMatrix;
};
