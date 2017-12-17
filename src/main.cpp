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
#include "LightEffectMatrixCircEQ.hpp"
#include "LightEffectMatrixTV.hpp"
#include "LightEffectMatrixExplode.hpp"
#include "LightEffectMatrixCircSpec.hpp"

using namespace std;

int main() {
	std::shared_ptr<LightEffect> matrixEQ, matrixCircEQ, soundSolid, stripEQ,
		stripSmoothEQ, matrixTV, matrixExplode, matrixCircSpec, visualizer;

	auto audioDevice = make_shared<AudioDevice>(AudioDevice::DEFAULT_DEVICE, 48000, 1024);
	auto spectrumAnalyzer = make_shared<SpectrumAnalyzer>(audioDevice, 32.7032, 16744.0384, 3, 4096);

	//auto display = std::make_shared<Display>();
	//matrixTV = make_shared<LightEffectMatrixTV>(display);
	
	matrixEQ = make_shared<LightEffectMatrixEQ>(spectrumAnalyzer);
	matrixEQ->setParameter("width", 32);
	matrixEQ->setParameter("height", 24);
	matrixEQ->setParameter("interleave", true);
	
	matrixCircEQ = make_shared<LightEffectMatrixCircEQ>(spectrumAnalyzer);
	matrixCircEQ->setParameter("width", 32);
	matrixCircEQ->setParameter("height", 24);
	matrixCircEQ->setParameter("band count", 6.);
	matrixCircEQ->setParameter("multiplier", 1.5);
	matrixCircEQ->setParameter("interleave", true);
	
	matrixCircSpec = make_shared<LightEffectMatrixCircSpec>(spectrumAnalyzer);
	matrixCircSpec->setParameter("width", 32);
	matrixCircSpec->setParameter("height", 24);
	matrixCircSpec->setParameter("band count", 6.);
	matrixCircSpec->setParameter("multiplier", 1.);
	matrixCircSpec->setParameter("interleave", true);

	matrixExplode = make_shared<LightEffectMatrixExplode>(spectrumAnalyzer);
	matrixExplode->setParameter("width", 32);
	matrixExplode->setParameter("height", 24);
	matrixExplode->setParameter("interleave", true);
	matrixExplode->setParameter("color filter strength", 0.5);

	visualizer = make_shared<LightEffectMatrixCircEQ>(spectrumAnalyzer);
	visualizer->setParameter("width", 128);
	visualizer->setParameter("height", 72);
	visualizer->setParameter("interleave", true);
	visualizer->setParameter("band count", 6.);
	
	soundSolid = make_shared<LightEffectSoundSolid>(spectrumAnalyzer);
	soundSolid->setParameter("color filter strength", 0.5);
	//soundSolid = make_shared<LightEffectFade>();
	//soundSolid->setParameter("db scaler", 500.);
	//soundSolid->setParameter("db factor", 2.);
	//soundSolid->setParameter("average factor", 1.);
	//soundSolid->setParameter("noise floor", 60.);

	stripEQ = make_shared<LightEffectStripEQ>(spectrumAnalyzer);
	stripEQ->setParameter("multiplier", 3.);
	
	stripSmoothEQ = make_shared<LightEffectStripEQ>(spectrumAnalyzer);
	stripSmoothEQ->setParameter("reverse", true);
	stripSmoothEQ->setParameter("smooth", true);
	stripSmoothEQ->setParameter("multiplier", 3.);

	Rhopalia rhopalia;
	rhopalia.addListener(LightHub::ListenerType::LightDiscover,
	[&matrixEQ, &matrixCircEQ, &soundSolid, &stripEQ, &stripSmoothEQ, &matrixTV,
		&matrixExplode, &matrixCircSpec, &visualizer]
		(std::shared_ptr<Light> light) {
		std::cout << "[Info] Light discovered: " << light->getName()
			<< " with " << light->getSize() << " LEDs" << std::endl;
		
		if(light->getName() == "LightNode - Visualizer") {
			if(visualizer->addLight(light)) {
				std::cout << "\tLight added to effect 'visualizer'\n" << std::endl;
			}
			else {
				std::cout << "\tFailed to add light to effect\n" << std::endl;
			}
		}
		else if(matrixEQ->addLight(light)) {
			std::cout << "\tLight added to effect 'Matrix EQ'\n" << std::endl;
		}
		else if((light->getSize() == 208) && stripEQ->addLight(light)) {
			std::cout << "\tLight added to effect 'Strip EQ'\n" << std::endl;
		}
		else if(stripSmoothEQ->addLight(light)) {
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
	rhopalia.addEffect(matrixCircEQ);
	rhopalia.addEffect(soundSolid);
	rhopalia.addEffect(stripEQ);
	rhopalia.addEffect(stripSmoothEQ);
	rhopalia.addEffect(matrixExplode);
	rhopalia.addEffect(matrixCircSpec);
	rhopalia.addEffect(visualizer);

	audioDevice->startStream();

	while(true) {
		rhopalia.run();
	}

	return 0;
}
