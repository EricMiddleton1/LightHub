#pragma once

#include <LightEffect.hpp>

#include <string>

class LightEffectMatrixClock : public LightEffect
{
public:
	LightEffectMatrixClock();

private:
	virtual void tick() override;
	virtual void updateStrip(std::shared_ptr<LightStrip>) override;

	std::string timeStr;
};
