#pragma once

#include "LightEffect.hpp"
#include "Color.hpp"

class LightEffectSolid : public LightEffect
{
public:
	LightEffectSolid();

private:
	virtual bool validateLight(const std::shared_ptr<Light>&) override;
	virtual void tick() override;
	virtual void updateLight(std::shared_ptr<Light>&) override;
};
