#include "LightEffectSoundSolid.hpp"


LightEffectSoundSolid::LightEffectSoundSolid(
	std::shared_ptr<SpectrumAnalyzer> _spectrumAnalyzer,
	const SoundColorSettings& settings, LightEffectSoundSolid::Channel _channel)
	:	ILightEffect({LightStrip::Type::Analog, LightStrip::Type::Digital})
	,	spectrumAnalyzer{_spectrumAnalyzer}
	,	soundColor(settings)
	,	channel{_channel} {
	
}

void LightEffectSoundSolid::tick() {

	switch(channel) {
		case LightEffectSoundSolid::Channel::Left:
			c = soundColor.getColor(spectrumAnalyzer->getLeftSpectrum());
		break;

		case LightEffectSoundSolid::Channel::Center:
			c = soundColor.getColor(spectrumAnalyzer->getCenterSpectrum());
		break;

		case LightEffectSoundSolid::Channel::Right:
			c = soundColor.getColor(spectrumAnalyzer->getRightSpectrum());
		break;
	}
}

void LightEffectSoundSolid::updateStrip(std::shared_ptr<LightStrip> strip) {
	auto buffer = strip->getBuffer();

	buffer->setAll(c);
}
