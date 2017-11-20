#include <memory>
#include <thread>
#include <chrono>

#include "Rhopalia.hpp"
#include "LightEffectSolid.hpp"
#include "LightEffectFade.hpp"
#include "LightEffectSoundSolid.hpp"
#include "LightEffectStripEQ.hpp"
#include "LightEffectStripBeat.hpp"
#include "LightEffectMatrixEQ.hpp"

using namespace std;

int main() {
	std::shared_ptr<LightEffect> matrixEQ, soundSolid, stripEQ, stripBeat;

	auto audioDevice = make_shared<AudioDevice>(AudioDevice::DEFAULT_DEVICE, 48000, 1024);
	auto spectrumAnalyzer = make_shared<SpectrumAnalyzer>(audioDevice, 32.7032, 16744.0384, 3, 4096);

	matrixEQ = make_shared<LightEffectMatrixEQ>(spectrumAnalyzer);
	matrixEQ->setParameter("width", 32);
	matrixEQ->setParameter("height", 24);
	matrixEQ->setParameter("interleave", true);

	soundSolid = make_shared<LightEffectSoundSolid>(spectrumAnalyzer);
	soundSolid->setParameter("color filter strength", 0.5);
	//soundSolid = make_shared<LightEffectFade>();
	//soundSolid->setParameter("db scaler", 500.);
	//soundSolid->setParameter("db factor", 2.);
	//soundSolid->setParameter("average factor", 1.);
	//soundSolid->setParameter("noise floor", 60.);

	stripEQ = make_shared<LightEffectStripEQ>(spectrumAnalyzer);
	stripBeat = make_shared<LightEffectStripBeat>(spectrumAnalyzer);

	Rhopalia rhopalia;
	rhopalia.addListener(LightHub::ListenerType::LightDiscover,
	[&matrixEQ, &soundSolid, &stripEQ, &stripBeat](std::shared_ptr<Light> light) {
		std::cout << "[Info] Light discovered: " << light->getName()
			<< " with " << light->getSize() << " LEDs" << std::endl;

		if(matrixEQ->addLight(light)) {
			std::cout << "\tLight added to effect 'Matrix EQ'\n" << std::endl;
		}
		else if((light->getSize() == 208) && stripEQ->addLight(light)) {
			std::cout << "\tLight added to effect 'Strip EQ'\n" << std::endl;
		}
		else if(stripBeat->addLight(light)) {
			std::cout << "\tLight added to effect 'Strip Beat'\n" << std::endl;
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
	rhopalia.addEffect(stripBeat);

	audioDevice->startStream();

	while(true) {
		rhopalia.run();
	}

	return 0;
}
