#pragma once

#include <LightEffect.hpp>

#include <string>

class LightEffectMatrixClock : public LightEffect
{
public:
	LightEffectMatrixClock();

private:
	virtual bool validateLight(const std::shared_ptr<Light>& light) override;
	virtual void tick() override;
	virtual void updateLight(std::shared_ptr<Light>&) override;

	std::string timeStr;
};
