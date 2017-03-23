#include "LightEffectSoundSolid.hpp"


LightEffectSoundSolid::LightEffectSoundSolid(
	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer,
	const SoundColorSettings& settings, LightEffectSoundSolid::Channel _channel)
	:	ILightEffect({LightStrip::Type::Analog, LightStrip::Type::Digital})
	,	soundColor(spectrumAnalyzer, settings)
	,	channel{_channel} {
	
}

void LightEffectSoundSolid::tick() {
	switch(channel) {
		case LightEffectSoundSolid::Channel::Left:
			c = soundColor.getLeftColor();
		break;

		case LightEffectSoundSolid::Channel::Center:
			c = soundColor.getCenterColor();
		break;

		case LightEffectSoundSolid::Channel::Right:
			c = soundColor.getRightColor();
		break;
	}
}

void LightEffectSoundSolid::updateStrip(std::shared_ptr<LightStrip> strip) {
	auto buffer = strip->getBuffer();

	buffer->setAll(c);
}
