#pragma once

#include <iostream> //For debugging

#include "ILightEffect.hpp"
#include "Color.hpp"

class LightEffectSolid : public ILightEffect
{
public:
	LightEffectSolid();
	LightEffectSolid(const Color& initialColor);

	virtual void addNode(const std::shared_ptr<LightNode>&) override;

	void setColor(const Color& c);
	Color getColor() const;

private:
	virtual void update() override;
	void slotStateChange(LightNode*, LightNode::State_e);

	//single color for all attached lights
	Color color;
};
