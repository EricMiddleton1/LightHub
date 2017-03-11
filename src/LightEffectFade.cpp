#include "LightEffectFade.hpp"


LightEffectFade::LightEffectFade(float brightness, float speed)
	:	ILightEffect({LightStrip::Type::Analog, LightStrip::Type::Digital})
	,	brightness{brightness}
	,	speed{speed}
	,	hue{0.}{

}

void LightEffectFade::tick() {
	hue = std::fmod(hue + speed, 360.f);
}

void LightEffectFade::updateStrip(std::shared_ptr<LightStrip> strip) {
	auto buffer = strip->getBuffer();
	buffer->setAll(Color::HSV(hue, 1.f, brightness));
}
