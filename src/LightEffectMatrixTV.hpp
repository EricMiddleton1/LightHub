#pragma once

#include <vector>
#include <memory>

#include "LightEffect.hpp"
#include "Display.hpp"

class LightEffectMatrixTV : public LightEffect
{
public:
	LightEffectMatrixTV(const std::shared_ptr<Display>& display);

private:
	bool validateLight(const std::shared_ptr<Light>&) override;
	void tick() override;
	void updateLight(std::shared_ptr<Light>& strip) override;

	std::shared_ptr<Display> display;
};
