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
	virtual void tick() override;
	virtual void updateStrip(std::shared_ptr<LightStrip>) override;

	double pos;
};
