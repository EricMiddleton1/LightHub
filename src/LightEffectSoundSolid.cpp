#include "LightEffectSoundSolid.hpp"


LightEffectSoundSolid::LightEffectSoundSolid(
	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer,
	const SoundColorSettings& settings)
	:	soundColor(spectrumAnalyzer, settings) {
	
}

LightEffectSoundSolid::~LightEffectSoundSolid() {

}

void LightEffectSoundSolid::update() {
	Color left, center, right;

	soundColor.getColor(&left, &center, &right);

	for(auto& node : nodes) {
		node->getLightStrip().setAll(left);
		node->releaseLightStrip();
	}
}
