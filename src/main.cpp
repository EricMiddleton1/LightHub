#include <memory>
#include <thread>
#include <chrono>

#include "Rhopalia.hpp"
#include "LightEffectSolid.hpp"
#include "LightEffectFade.hpp"
#include "LightEffectSoundSolid.hpp"
#include "LightEffectStripEQ.hpp"
#include "LightEffectMatrixEQ.hpp"

using namespace std;

int main() {
	std::shared_ptr<LightEffect> lightEffect;

	auto audioDevice = make_shared<AudioDevice>(AudioDevice::DEFAULT_DEVICE, 48000, 1024);
	auto spectrumAnalyzer = make_shared<SpectrumAnalyzer>(audioDevice, 32.7032, 16744.0384, 3, 4096);

	lightEffect = make_shared<LightEffectMatrixEQ>(spectrumAnalyzer);
	lightEffect->setParameter("width", 32);
	lightEffect->setParameter("height", 24);

	Rhopalia rhopalia;
	rhopalia.addListener(LightHub::ListenerType::LightDiscover,
	[&lightEffect](std::shared_ptr<Light> light) {
		std::cout << "[Info] Light discovered: " << light->getName()
			<< " with " << light->getSize() << " LEDs" << std::endl;

		if(!lightEffect->addLight(light)) {
			std::cout << "[Error] Failed to add light to effect" << std::endl;
		}
	});

	rhopalia.addEffect(lightEffect);
	audioDevice->startStream();

	while(true) {
		rhopalia.run();
	}

	return 0;
}
