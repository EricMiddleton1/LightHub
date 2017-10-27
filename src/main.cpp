#include <memory>
#include <thread>
#include <chrono>

#include "Rhopalia.hpp"
#include "LightEffectSolid.hpp"
#include "LightEffectFade.hpp"
#include "LightEffectSoundSolid.hpp"

using namespace std;

int main() {
	Rhopalia rhopalia;
	
	auto audioDevice = make_shared<AudioDevice>(AudioDevice::DEFAULT_DEVICE, 48000, 512);
	auto lightEffect = make_shared<LightEffectSoundSolid>(
		make_shared<SpectrumAnalyzer>(audioDevice, 30, 20000, 3, 4096),
			LightEffectSoundSolid::Channel::Left);

	rhopalia.addEffect(lightEffect);
	rhopalia.addListener(LightHub::ListenerType::LightDiscover,
		[&lightEffect](std::shared_ptr<Light> light) {
			std::cout << "[Info] Light discovered: " << light->getName()
				<< " with " << light->getSize() << " LEDs" << std::endl;

			lightEffect->addLight(light);
		});

	while(true) {
		rhopalia.run();
	}

	return 0;

}
