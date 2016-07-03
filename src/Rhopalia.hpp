#pragma once

#include <vector>
#include <memory>

#include "LightHub.hpp"
#include "ILightEffect.hpp"


class Rhopalia
{
public:



private:
	LightHub hub;
	std::vector<ILightEffect> lightEffects;
};
