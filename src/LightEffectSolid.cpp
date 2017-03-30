#include "LightEffectSolid.hpp"


LightEffectSolid::LightEffectSolid()
	:	LightEffect({LightStrip::Type::Analog, LightStrip::Type::Digital},
		{{"color", Color()}}) {
}

void LightEffectSolid::tick() {
}

void LightEffectSolid::updateStrip(std::shared_ptr<LightStrip> strip) {
	strip->getBuffer()->setAll(getParameter("color").getValue().getColor());
}
