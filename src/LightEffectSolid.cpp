#include "LightEffectSolid.hpp"


LightEffectSolid::LightEffectSolid()
	:	LightEffect({{"color", Color()}}) {
}

bool LightEffectSolid::validateLight(const std::shared_ptr<Light>& light) {
	return true;
}

void LightEffectSolid::tick() {
}

void LightEffectSolid::updateLight(std::shared_ptr<Light>& light) {
	light->getBuffer()->setAll(getParameter("color").getValue().getColor());
}
