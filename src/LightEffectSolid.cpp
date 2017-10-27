#include "LightEffectSolid.hpp"


LightEffectSolid::LightEffectSolid()
	:	LightEffect({{"color", Color()}}) {
}

void LightEffectSolid::tick() {
}

void LightEffectSolid::updateLight(std::shared_ptr<Light>& light) {
	light->getBuffer().setAll(getParameter("color").getValue().getColor());
}
