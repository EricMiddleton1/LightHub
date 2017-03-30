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

#define HUB_TO_NODE_PORT	54923
#define NODE_TO_HUB_PORT	54924

void slotNodeDiscover(std::shared_ptr<LightNode>);

void slotNodeStateChange(LightNode*, LightNode::State, LightNode::State);

std::shared_ptr<LightEffect> leftEffect, centerEffect, rightEffect, bassEffect;
std::shared_ptr<LightEffect> digitalEffect;
std::shared_ptr<LightEffect> matrixEffect, visualizerEffect;

void printNodes(Rhopalia&);

int main() {
	//Create an audio device
	std::shared_ptr<AudioDevice> audioDevice =
		std::make_shared<AudioDevice>(AudioDevice::DEFAULT_DEVICE, 48000, 1024);

	//Create a spectrum analyzer
	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer =
		std::make_shared<SpectrumAnalyzer>(audioDevice,32.7032,16744.0384,3, 4096);

	//Left/Right full range sound effects
	leftEffect = std::make_shared<LightEffectSoundSolid>(spectrumAnalyzer,
		LightEffectSoundSolid::Channel::Left);
	rightEffect = std::make_shared<LightEffectSoundSolid>(spectrumAnalyzer,
		LightEffectSoundSolid::Channel::Right);
	centerEffect = std::make_shared<LightEffectSoundSolid>(spectrumAnalyzer,
		LightEffectSoundSolid::Channel::Center);
	bassEffect = std::make_shared<LightEffectSoundSolid>(spectrumAnalyzer,
		LightEffectSoundSolid::Channel::Center);

	bassEffect->setParameter("bass freq", 120.);
	bassEffect->setParameter("end frequency", 120.);
	bassEffect->setParameter("bass boost", 6.);
	bassEffect->setParameter("db scaler", 200.);
	bassEffect->setParameter("average factor", 40.);
	bassEffect->setParameter("noise floor", 60.);
	bassEffect->setParameter("average filter strength", 0.4);
	bassEffect->setParameter("color filter strength", 0.4);
	bassEffect->setParameter("threshold", 0.125);

	digitalEffect = std::make_shared<LightEffectStripEQ>(spectrumAnalyzer);
	matrixEffect = std::make_shared<LightEffectMatrixEQ>(spectrumAnalyzer);

	Rhopalia controller;

	controller.addListener(LightHub::NODE_DISCOVER, &slotNodeDiscover);

	controller.addEffect(leftEffect);
	controller.addEffect(centerEffect);
	controller.addEffect(rightEffect);
	controller.addEffect(bassEffect);
	controller.addEffect(digitalEffect);
	controller.addEffect(matrixEffect);
	
	//Start the audio device
	audioDevice->startStream();
	
	//Everything is handled by other threads now
	for(;;) {
		std::this_thread::sleep_for(std::chrono::seconds(10));
/*
		char msg[1024];

		std::cout << ">";
		std::cin.getline(msg, sizeof(msg));

		matrixEffect->setParameter("text", std::string(msg));
*/
	}

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
						centerEffect->addStrip(strip);
					else if(analogCount == 2)
						rightEffect->addStrip(strip);
					else
						bassEffect->addStrip(strip);

					analogCount++;

					std::cout << "\tAnalog Strip (" << analogCount << ")" << std::endl;
				break;

				case LightStrip::Type::Digital:
					digitalEffect->addStrip(strip);

					std::cout << "\tDigital strip (" << strip->getSize() << ")"  << std::endl;
				break;

				case LightStrip::Type::Matrix:
					matrixEffect->addStrip(strip);

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
