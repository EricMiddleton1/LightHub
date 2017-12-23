#include "LightEffectFade.hpp"


LightEffectFade::LightEffectFade()
	:	LightEffect{{{"brightness", 1., Parameter::ValidatorRange(0., 1.)},
		{"speed", 1., Parameter::ValidatorGreater(0.)}}}
	,	hue{0.}{
}

bool LightEffectFade::validateLight(const std::shared_ptr<Light>& light) {
	return true;
}

void LightEffectFade::tick() {
	hue = std::fmod(hue + getParameter("speed").getValue().getDouble(), 360.f);
}

void LightEffectFade::updateLight(std::shared_ptr<Light>& light) {
	auto buffer = light->getBuffer();
	buffer->setAll(Color::HSV(hue*255.f/360.f, 255,
		255*getParameter("brightness").getValue().getDouble()));
}
