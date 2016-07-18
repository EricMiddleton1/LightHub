#pragma once

#include "ILightEffect.hpp"
#include "Color.hpp"

class LightEffectSolid : public ILightEffect
{
public:
	LightEffectSolid();
	LightEffectSolid(const Color& initialColor);

	void setColor(const Color& c);
	Color getColor() const;

private:
};
