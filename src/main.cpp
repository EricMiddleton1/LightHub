#include <thread>
#include <chrono>
#include <memory>
#include <iostream>
#include <algorithm>

#include "Rhopalia.hpp"

#include "AudioDevice.hpp"
#include "SpectrumAnalyzer.hpp"

#include "LightEffectFade.hpp"
#include "LightEffectSolid.hpp"
#include "LightEffectSoundSolid.hpp"
#include "LightEffectStripEQ.hpp"
#include "LightEffectMatrixEQ.hpp"
#include "LightEffectMatrixText.hpp"
#include "LightEffectMatrixClock.hpp"
#include "LightEffectDigitalTV.hpp"
#include "LightEffectMatrixTV.hpp"

#define HUB_TO_NODE_PORT	54923
#define NODE_TO_HUB_PORT	54924

void slotNodeDiscover(std::shared_ptr<LightNode>);

void slotNodeStateChange(LightNode*, LightNode::State, LightNode::State);

std::shared_ptr<LightEffect> fadeEffect;
std::shared_ptr<LightEffect> leftEffect, centerEffect, rightEffect, bassEffect;
std::shared_ptr<LightEffect> digitalEffect;
std::shared_ptr<LightEffect> matrixEffect, visualizerEffect;
std::shared_ptr<LightEffect> tvEffect, matrixTVEffect;

void printNodes(Rhopalia&);

int main() {
	//Create an audio device
	std::shared_ptr<AudioDevice> audioDevice =
		std::make_shared<AudioDevice>(AudioDevice::DEFAULT_DEVICE, 48000, 1024);

	//Create a spectrum analyzer
	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer =
		std::make_shared<SpectrumAnalyzer>(audioDevice,32.7032,16744.0384,3, 4096);
	
	fadeEffect = std::make_shared<LightEffectFade>();
	fadeEffect->setParameter("brightness", 0.25);

	//Left/Right full range sound effects
	leftEffect = std::make_shared<LightEffectSoundSolid>(spectrumAnalyzer,
		LightEffectSoundSolid::Channel::Left);
	rightEffect = std::make_shared<LightEffectSoundSolid>(spectrumAnalyzer,
		LightEffectSoundSolid::Channel::Right);
	centerEffect = std::make_shared<LightEffectSoundSolid>(spectrumAnalyzer,
		LightEffectSoundSolid::Channel::Center);
	bassEffect = std::make_shared<LightEffectSoundSolid>(spectrumAnalyzer,
		LightEffectSoundSolid::Channel::Center);

	//leftEffect->setParameter("start frequency", 80.);
	//rightEffect->setParameter("start frequency", 80.);
	//leftEffect->setParameter("bass boost", 0.);
	//rightEffect->setParameter("bass boost", 0.);
	//leftEffect->setParameter("average filter strength", 0.5);
	//rightEffect->setParameter("average filter strength", 0.5);
	//leftEffect->setParameter("average factor", 1.5);
	//rightEffect->setParameter("average factor", 1.5);
	//leftEffect->setParameter("db scaler", 200.);
	//rightEffect->setParameter("db scaler", 200.);
	//leftEffect->setParameter("change factor", 0.5);
	//rightEffect->setParameter("change factor", 0.5);
	
	bassEffect->setParameter("bass freq", 120.);
	bassEffect->setParameter("end frequency", 120.);
	bassEffect->setParameter("bass boost", 6.);
	bassEffect->setParameter("db scaler", 200.);
	bassEffect->setParameter("average factor", 1.5);
	bassEffect->setParameter("noise floor", 50.);
	bassEffect->setParameter("average filter strength", 0.2);
	bassEffect->setParameter("color filter strength", 0.6);
	bassEffect->setParameter("threshold", 0.1);

	digitalEffect = std::make_shared<LightEffectStripEQ>(spectrumAnalyzer);
	matrixEffect = std::make_shared<LightEffectMatrixEQ>(spectrumAnalyzer);

	//auto display = std::make_shared<Display>();
	//std::cout << "[Info] Display (" << display->getWidth() << ", " << display->getHeight()
		//<< ")" << std::endl;

	//tvEffect = std::make_shared<LightEffectDigitalTV>(display);
	//matrixTVEffect = std::make_shared<LightEffectMatrixTV>(display);

	Rhopalia controller;

	controller.addListener(LightHub::NODE_DISCOVER, &slotNodeDiscover);
	
	controller.addEffect(fadeEffect);
	controller.addEffect(leftEffect);
	controller.addEffect(centerEffect);
	controller.addEffect(rightEffect);
	controller.addEffect(bassEffect);
	controller.addEffect(digitalEffect);
	controller.addEffect(matrixEffect);
	//controller.addEffect(tvEffect);
	//controller.addEffect(matrixTVEffect);

	//Start the audio device
	audioDevice->startStream();
	
	controller.run();

	return 0;
}

void slotNodeDiscover(std::shared_ptr<LightNode> node) {
	node->addListener(LightNode::ListenerType::STATE_CHANGE, slotNodeStateChange);
	
	std::cout << "[Info] New node discovered \"" << node->getName() << "\":" << std::endl;

	unsigned int analogCount = 0;
	std::for_each(node->stripBegin(), node->stripEnd(),
		[&analogCount](std::shared_ptr<LightStrip>& strip) {
			switch(strip->getType()) {
				case LightStrip::Type::Analog:
					if(analogCount == 0)
						leftEffect->addStrip(strip);
					else if(analogCount == 1)
						rightEffect->addStrip(strip);
					else if(analogCount == 2)
						centerEffect->addStrip(strip);
					else
						bassEffect->addStrip(strip);

					analogCount++;

					std::cout << "\tAnalog Strip (" << analogCount << ")" << std::endl;
				break;

				case LightStrip::Type::Digital:
					fadeEffect->addStrip(strip);
					//digitalEffect->addStrip(strip);
					//tvEffect->addStrip(strip);

					std::cout << "\tDigital strip (" << strip->getSize() << ")"  << std::endl;
				break;

				case LightStrip::Type::Matrix:
					matrixEffect->addStrip(strip);
					//matrixTVEffect->addStrip(strip);

					std::cout << "\tMatrix" << std::endl;
				break;
				
				default:
					std::cout << "\tUnknown strip (" << strip->getSize() << ")" << std::endl;
				break;
			}
		});
}

void slotNodeStateChange(LightNode* node, LightNode::State,
	LightNode::State newState) {

	std::cout << "[Info] slotNodeStateChange: Node '" << node->getName()
		<< "' state changed to " << LightNode::stateToString(newState)
		<< std::endl;
}

void printNodes(Rhopalia& controller) {
	int i = 0;
	std::for_each(controller.nodeBegin(), controller.nodeEnd(),
		[&i](std::shared_ptr<LightNode> node) {
			std::cout << "[" << i++ << "]\t" << node->getName() << std::endl;
		});
}
