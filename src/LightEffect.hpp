#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <utility>
#include <mutex>

#include "Exception.hpp"
#include "LightStrip.hpp"
#include "Parameter.hpp"
#include "ConfigurableObject.hpp"

//Forward declaration
class Rhopalia;

class LightEffect : public ConfigurableObject
{
public:
	static const uint16_t EXCEPTION_LIGHT_EFFECT_STRIP_NOT_FOUND = 0x0020;
	static const uint16_t EXCEPTION_LIGHT_EFFECT_UNSUPPORTED_TYPE = 0x0021;
	static const uint16_t EXCEPTION_LIGHT_EFFECT_STRIP_ALREADY_CONNECTED = 0x0022;

	LightEffect(const std::vector<LightStrip::Type>& supportedTypes,
		const std::vector<Parameter>& parameters);
	virtual ~LightEffect();

	void addStrip(const std::shared_ptr<LightStrip>&);
	void removeStrip(size_t id);

protected:
	virtual void tick() = 0;
	virtual void updateStrip(std::shared_ptr<LightStrip>) = 0;

	std::function<void(std::pair<size_t, std::weak_ptr<LightStrip>>&)> onAdd, onRemove;

private:
	friend class Rhopalia;

	void update();

	std::vector<std::pair<size_t, std::weak_ptr<LightStrip>>> strips;
	mutable std::mutex stripsMutex;

	std::vector<LightStrip::Type> supportedTypes;
};
