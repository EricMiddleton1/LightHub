#pragma once

#include <string>
#include <mutex>

#include "LightEffect.hpp"
#include "Color.hpp"

class LightEffectMatrixText : public LightEffect
{
public:
	LightEffectMatrixText();

private:
	virtual bool validateLight(const std::shared_ptr<Light>&) override;
	virtual void tick() override;
	virtual void updateLight(std::shared_ptr<Light>&) override;

	double pos;
};
