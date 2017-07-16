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
	void tick() override;
	void updateStrip(std::shared_ptr<LightStrip> strip) override;

	std::shared_ptr<Display> display;
	
	Display::Coordinate width, height;
	std::vector<Color> edge;
};
