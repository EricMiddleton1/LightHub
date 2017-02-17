#include "LightEffectSoundMove.hpp"


LightEffectSoundMove::LightEffectSoundMove(
	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer,
	const SoundColorSettings& settings)
	:	soundColor(spectrumAnalyzer, settings) {
	
}

LightEffectSoundMove::~LightEffectSoundMove() {

}

void LightEffectSoundMove::update() {
	Color left, center, right;

	soundColor.getColor(&left, &center, &right);

	for(auto& node : nodes) {
		//node->getLightStrip().setAll(left);
		auto& strip = node->getLightStrip();

		for(size_t i = (strip.getSize() - 1); i > 0; --i) {
			strip.setPixel(i, strip.getPixel(i-1));
		}
		strip.setPixel(0, left);

		node->releaseLightStrip();
	}
}
