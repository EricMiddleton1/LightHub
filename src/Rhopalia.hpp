#pragma once

#include <vector>
#include <memory>

#include "LightHub.hpp"
#include "ILightEffect.hpp"


class Rhopalia
{
public:
	Rhopalia();
	~Rhopalia();




private:
	LightHub hub;
	std::vector<ILightEffect> lightEffects;
};
