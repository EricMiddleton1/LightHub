#pragma once

#include "LightEffect.hpp"
#include "Color.hpp"

class LightEffectFade : public LightEffect
{
public:
	LightEffectFade();

private:
	virtual void tick() override;
	virtual void updateStrip(std::shared_ptr<LightStrip>) override;

	//single color for all attached lights
	Color color;

	double hue;
};
