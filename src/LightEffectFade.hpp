#pragma once

#include "LightEffect.hpp"
#include "Color.hpp"

class LightEffectFade : public LightEffect
{
public:
	LightEffectFade();

private:
	virtual void tick() override;
	virtual void updateLight(std::shared_ptr<Light>&) override;

	//single color for all attached lights
	Color color;

	double hue;
};
