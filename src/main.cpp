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
	std::shared_ptr<LightEffect> matrixEQ, soundSolid, stripEQ;

	auto audioDevice = make_shared<AudioDevice>(AudioDevice::DEFAULT_DEVICE, 48000, 1024);
	auto spectrumAnalyzer = make_shared<SpectrumAnalyzer>(audioDevice, 32.7032, 16744.0384, 3, 4096);

	matrixEQ = make_shared<LightEffectMatrixEQ>(spectrumAnalyzer);
	matrixEQ->setParameter("width", 32);
	matrixEQ->setParameter("height", 24);
	matrixEQ->setParameter("interleave", false);

	soundSolid = make_shared<LightEffectSoundSolid>(spectrumAnalyzer);
	//soundSolid->setParameter("db scaler", 500.);
	//soundSolid->setParameter("db factor", 2.);
	//soundSolid->setParameter("average factor", 1.);
	//soundSolid->setParameter("noise floor", 60.);

	stripEQ = make_shared<LightEffectStripEQ>(spectrumAnalyzer);

	Rhopalia rhopalia;
	rhopalia.addListener(LightHub::ListenerType::LightDiscover,
	[&matrixEQ, &soundSolid, &stripEQ](std::shared_ptr<Light> light) {
		std::cout << "[Info] Light discovered: " << light->getName()
			<< " with " << light->getSize() << " LEDs" << std::endl;

		if(matrixEQ->addLight(light)) {
			std::cout << "\tLight added to effect 'Matrix EQ'\n" << std::endl;
		}
		else if((light->getSize() > 1) && stripEQ->addLight(light)) {
			std::cout << "\tLight added to effect 'Strip EQ'\n" << std::endl;
		}
		else if(soundSolid->addLight(light)) {
			std::cout << "\tLight added to effect 'Sound Solid'\n" << std::endl;
		}
		else {
			std::cerr << "\tFailed to add light to an effect\n" << std::endl;
		}
	});

	rhopalia.addEffect(matrixEQ);
	rhopalia.addEffect(soundSolid);
	rhopalia.addEffect(stripEQ);

	audioDevice->startStream();

	while(true) {
		rhopalia.run();
	}

	return 0;
}
