#include "LightEffectFade.hpp"


LightEffectFade::LightEffectFade()
	:	LightEffect({LightStrip::Type::Analog, LightStrip::Type::Digital},
		{{"brightness", 1., Parameter::ValidatorRange(0., 1.)},
		{"speed", 1., Parameter::ValidatorGreater(0.)}})
	,	hue{0.}{

}

void LightEffectFade::tick() {
	hue = std::fmod(hue + getParameter("speed").getValue().getNumber(), 360.f);
}

void LightEffectFade::updateStrip(std::shared_ptr<LightStrip> strip) {
	auto buffer = strip->getBuffer();
	buffer->setAll(Color::HSV(hue, 1.f, getParameter("brightness").getValue().getNumber()));
}
