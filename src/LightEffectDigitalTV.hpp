#pragma once

#include <vector>
#include <memory>

#include "LightEffect.hpp"
#include "Color.hpp"
#include "Display.hpp"

class LightEffectDigitalTV : public LightEffect
{
public:
	LightEffectDigitalTV(const std::shared_ptr<Display>& display);

private:
	bool validateLight(const std::shared_ptr<Light>& light) override;
	void tick() override;
	void updateLight(std::shared_ptr<Light>& light) override;

	std::shared_ptr<Display> display;
	
	Display::Coordinate width, height;
	std::vector<Color> edge;
};
