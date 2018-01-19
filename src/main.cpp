#include <memory>
#include <thread>
#include <chrono>

#include "Rhopalia.hpp"
#include "Matrix.hpp"

#include "LightEffectSolid.hpp"
#include "LightEffectFade.hpp"
#include "LightEffectSoundSolid.hpp"
#include "LightEffectStripEQ.hpp"
#include "LightEffectMatrixEQ.hpp"
#include "LightEffectMatrixTV.hpp"
#include "LightEffectMatrixExplode.hpp"
#include "LightEffectMatrixText.hpp"
#include "LightEffectMatrixClock.hpp"
#include "LightEffectMatrixCircSpec.hpp"
#include "LightEffectMatrixCircEQ.hpp"

using namespace std;

int main() {
	std::shared_ptr<LightEffect> matrixEQ, matrixCircEQ, soundSolid, stripEQ,
		stripSmoothEQ, matrixTV, matrixExplode, matrixCircSpec, visualizer, matrixText, fade,
		matrixClock;

	auto audioDevice = make_shared<AudioDevice>(AudioDevice::DEFAULT_DEVICE, 48000, 1024);
	auto spectrumAnalyzer = make_shared<SpectrumAnalyzer>(audioDevice, 32.7032,
		16744.0384, 3, 4096);

	fade = make_shared<LightEffectFade>();

	soundSolid = make_shared<LightEffectSoundSolid>(spectrumAnalyzer);
	soundSolid->setParameter("color filter strength", 0.5);

	stripEQ = make_shared<LightEffectStripEQ>(spectrumAnalyzer);
	stripEQ->setParameter("multiplier", 2.);
	
	stripSmoothEQ = make_shared<LightEffectStripEQ>(spectrumAnalyzer);
	stripSmoothEQ->setParameter("reverse", true);
	stripSmoothEQ->setParameter("smooth", true);
	stripSmoothEQ->setParameter("multiplier", 3.);

	matrixEQ = make_shared<LightEffectMatrixEQ>(spectrumAnalyzer);

	matrixExplode = make_shared<LightEffectMatrixExplode>(spectrumAnalyzer);
	matrixExplode->setParameter("color filter strength", 0.5);

	matrixText = make_shared<LightEffectMatrixText>();
	matrixText->setParameter("text", "Hello, World!"s);
	matrixText->setParameter("color", Color(0, 64, 0));
	matrixText->setParameter("speed", 15.);

	matrixClock = make_shared<LightEffectMatrixClock>();
	matrixClock->setParameter("color", Color(0, 0, 64));

	matrixCircSpec = make_shared<LightEffectMatrixCircSpec>(spectrumAnalyzer);
	matrixCircSpec->setParameter("band count", 6.);
	matrixCircSpec->setParameter("multiplier", 1.);

	matrixCircEQ = make_shared<LightEffectMatrixCircEQ>(spectrumAnalyzer);
	matrixCircEQ->setParameter("band count", 4.);
	matrixCircEQ->setParameter("multiplier", 1.5);

/*
	auto display = std::make_shared<Display>();
	matrixTV = make_shared<LightEffectMatrixTV>(display);
	
*/


	Rhopalia rhopalia;
	rhopalia.addListener(LightHub::ListenerType::LightDiscover,
	[&soundSolid, &stripEQ, &stripSmoothEQ, &matrixEQ, &matrixExplode, &matrixText,
		&matrixClock, &matrixCircSpec, &matrixCircEQ, &fade] (std::shared_ptr<Light> light) {

		if(Matrix::isMatrix(light)) {
			auto matrix = dynamic_pointer_cast<Matrix>(light);
			std::cout << "[Info] Matrix discovered: " << light->getName()
				<< " with " << static_cast<int>(matrix->getWidth()) << "x"
				<< static_cast<int>(matrix->getHeight()) << " resolution" << std::endl;
				
			if(light->getName() == "matrix") {
				matrixEQ->addLight(light);
				std::cout << "\tMatrix added to effect 'Matrix Explode'\n" << std::endl;
			}
			else if(light->getName() == "Little Matrix") {
				matrixExplode->addLight(light);
				std::cout << "\tMatrix added to effect 'Matrix Explode'\n" << std::endl;
			}
			else if(light->getName() == "Lamp") {
				matrixExplode->addLight(light);
				std::cout << "\tMatrix added to effect 'Matrix Explode'\n" << std::endl;
			}
			else {
				matrixExplode->addLight(light);
				std::cout << "\tMatrix added to effect 'Matrix Explode'\n" << std::endl;
			}

		/*
			if(light->getName() == "LightNode - Visualizer") {
				if(visualizer->addLight(light)) {
					std::cout << "\tMatrix added to effect 'visualizer'\n" << std::endl;
				}
				else {
					std::cout << "\tFailed to add light to effect\n" << std::endl;
				}
			}
			else if(light->getName() == "Test" && stripEQ->addLight(light)) {
				std::cout << "\tMatrix added to effect 'Test Effect'\n" << std::endl;
			}
			else if(matrixEQ->addLight(light)) {
				std::cout << "\tMatrix added to effect 'Matrix EQ'\n" << std::endl;
			}
			else {
				std::cout << "\tFailed to add matrix to effect\n" << std::endl;
			}
		*/
		}
		else {
			std::cout << "[Info] Light discovered: " << light->getName()
				<< " with " << light->getSize() << " LEDs" << std::endl;
			
			if((light->getSize() == 208) && stripEQ->addLight(light)) {
				std::cout << "\tLight added to effect 'Strip EQ'\n" << std::endl;
			}
			else if(stripSmoothEQ->addLight(light)) {
				std::cout << "\tLight added to effect 'Strip Beat'\n" << std::endl;
			}
/*
			else if(fade->addLight(light)) {
				std::cout << "\tLight added to effect 'Fade'\n" << std::endl;
			}
*/
			else if(soundSolid->addLight(light)) {
				std::cout << "\tLight added to effect 'Sound Solid'\n" << std::endl;
			}
			else {
				std::cerr << "\tFailed to add light to an effect\n" << std::endl;
			}
		}
	});


	rhopalia.addEffect(soundSolid);
	rhopalia.addEffect(stripEQ);
	rhopalia.addEffect(stripSmoothEQ);
	rhopalia.addEffect(matrixEQ);
	rhopalia.addEffect(matrixExplode);
	rhopalia.addEffect(matrixCircSpec);
	rhopalia.addEffect(matrixCircEQ);
	rhopalia.addEffect(matrixText);
	rhopalia.addEffect(fade);

	audioDevice->startStream();

	while(true) {
		rhopalia.run();
	}

	return 0;
}
