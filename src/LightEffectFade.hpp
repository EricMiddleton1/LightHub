#pragma once

#include <cmath>
#include <iostream> //For debugging

#include "ILightEffect.hpp"
#include "Color.hpp"

class LightEffectFade : public ILightEffect
{
public:
	LightEffectFade(float brightness, float speed);

	virtual void addNode(const std::shared_ptr<LightNode>&) override;

private:
	virtual void update() override;
	void slotStateChange(LightNode*, LightNode::State_e);

	float brightness, speed, hue;

	//single color for all attached lights
	Color color;
};
