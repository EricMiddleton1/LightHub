#pragma once

#include <cmath>
#include <iostream> //For debugging

#include "ILightEffect.hpp"
#include "Color.hpp"

class LightEffectFade : public ILightEffect
{
public:
	LightEffectFade(float brightness, float speed);

private:
	virtual void tick() override;
	virtual void updateStrip(std::shared_ptr<LightStrip>) override;

	float brightness, speed, hue;

	//single color for all attached lights
	Color color;
};
