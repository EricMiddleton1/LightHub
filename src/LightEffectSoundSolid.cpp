#include "LightEffectSoundSolid.hpp"


LightEffectSoundSolid::LightEffectSoundSolid(
	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer,
	const SoundColorSettings& settings)
	:	ILightEffect({LightStrip::Type::Analog, LightStrip::Type::Digital})
	,	soundColor(spectrumAnalyzer, settings) {
	
}

void LightEffectSoundSolid::tick() {
	soundColor.getColor(&left, &center, &right);
}

void LightEffectSoundSolid::updateStrip(std::shared_ptr<LightStrip> strip) {
	auto buffer = strip->getBuffer();

	buffer->setAll(left);
}
