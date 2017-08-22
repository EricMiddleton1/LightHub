#pragma once

#include <vector>
#include <memory>

#include "LightEffect.hpp"
#include "Color.hpp"
#include "Display.hpp"

class LightEffectMatrixTV : public LightEffect
{
public:
	LightEffectMatrixTV(const std::shared_ptr<Display>& display);

private:
	void tick() override;
	void updateStrip(std::shared_ptr<LightStrip> strip) override;

	std::shared_ptr<Display> display;
	std::unique_ptr<DisplayBuffer> displayBuffer;
};
