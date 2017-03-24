#include <thread>
#include <chrono>
#include <memory>
#include <iostream>
#include <algorithm>

#include "Rhopalia.hpp"

#include "AudioDevice.hpp"
#include "SpectrumAnalyzer.hpp"
#include "SoundColor.hpp"

#include "LightEffectFade.hpp"
#include "LightEffectSoundSolid.hpp"
#include "LightEffectSoundMove.hpp"
#include "LightEffectStripEQ.hpp"
#include "LightEffectMatrixEQ.hpp"
#include "LightEffectMatrixText.hpp"

#define IP_ADDR	"192.168.1.3"
#define HUB_TO_NODE_PORT	54923
#define NODE_TO_HUB_PORT	54924

void slotNodeDiscover(std::shared_ptr<LightNode>);

void slotNodeStateChange(LightNode*, LightNode::State, LightNode::State);

std::shared_ptr<ILightEffect> leftEffect, centerEffect, rightEffect, bulbEffect;
std::shared_ptr<ILightEffect> digitalEffect;
std::shared_ptr<ILightEffect> matrixEffect, visualizerEffect;


void printNodes(Rhopalia&);

int main() {
	//Create an audio device
	std::shared_ptr<AudioDevice> audioDevice =
		std::make_shared<AudioDevice>(AudioDevice::DEFAULT_DEVICE, 48000, 1024);

	//Create a spectrum analyzer
	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer =
		std::make_shared<SpectrumAnalyzer>(audioDevice,32.7032,16744.0384,3, 4096);

	//Configure SoundColor settings
	SoundColorSettings scs;
	scs.bassFreq = 150.;
	scs.trebbleFreq = 4000.;
	scs.bassBoost = 10.;
	scs.trebbleBoost = 0.;
	scs.fStart = 0;
	scs.fEnd = 20000;
	scs.dbScaler = 300;
	scs.dbFactor = 2.;
	scs.avgFactor = 1.;
	scs.noiseFloor = 60.;
	scs.avgFilterStrength = 0.4;
	scs.minSaturation = 0.7;
	scs.filterStrength = 0.7;
	scs.centerSpread = 0.5;
	scs.centerBehavior = SoundColorSettings::MONO;


	//Left/Right full range sound effects
	leftEffect = std::make_shared<LightEffectSoundSolid>(spectrumAnalyzer, scs,
		LightEffectSoundSolid::Channel::Left);
	rightEffect = std::make_shared<LightEffectSoundSolid>(spectrumAnalyzer, scs,
		LightEffectSoundSolid::Channel::Right);
	centerEffect = std::make_shared<LightEffectSoundSolid>(spectrumAnalyzer, scs,
		LightEffectSoundSolid::Channel::Center);


	//Mono bass sound effect
	scs.bassBoost = 6.;
	scs.fEnd = 120.;
	scs.bassFreq = scs.fEnd;
	scs.dbScaler = 225;
	scs.noiseFloor = 40.;
	scs.dbFactor = 2.;
	scs.avgFilterStrength = 0.0;
	scs.minSaturation = 0;
	scs.filterStrength = 0.4;
	bulbEffect = std::make_shared<LightEffectSoundSolid>(spectrumAnalyzer, scs,
		LightEffectSoundSolid::Channel::Center);

	digitalEffect = std::make_shared<LightEffectStripEQ>(spectrumAnalyzer);
	matrixEffect = std::make_shared<LightEffectMatrixEQ>(spectrumAnalyzer, 100);
	visualizerEffect = std::make_shared<LightEffectMatrixEQ>(spectrumAnalyzer, 100, true);
	std::shared_ptr<LightEffectMatrixText> textEffect
		(std::make_shared<LightEffectMatrixText>());
	//matrixEffect = textEffect;

	textEffect->setColor(Color(0, 0, 255));

	Rhopalia controller;

	controller.addListener(LightHub::NODE_DISCOVER, &slotNodeDiscover);

	controller.addEffect(leftEffect);
	controller.addEffect(centerEffect);
	controller.addEffect(rightEffect);
	controller.addEffect(bulbEffect);
	controller.addEffect(digitalEffect);
	controller.addEffect(matrixEffect);
	controller.addEffect(visualizerEffect);
	
	//Start the audio device
	audioDevice->startStream();
/*
	//Everything is handled by other threads now
	for(;;) {
		std::cout << ">";

		std::string input;
		std::cin >> input;

		if(input == "help") {
			std::cout << "Commands:\n"
				"nodes - List nodes\n"
				"wificonnect - Connect to WiFi station on node\n"
				"wifistartap - Start WiFi AP on node\n"
				<< std::endl;
		}
		else if(input == "nodes") {
			printNodes(controller);
		}
		else if(input == "wificonnect") {
			printNodes(controller);

			int index = 0;
			std::string ssid, psk, sec;

			std::cout << "Node Index: ";
			std::cin >> index;
			std::cout << "SSID: ";
			std::cin >> ssid;
			std::cout << "Does AP require a password (y/n): ";
			std::cin >> sec;

			if(sec == "y") {
				std::cout << "PSK: ";
				std::cin >> psk;
			}


			//TODO: check bounds
			auto node = *(controller.nodeBegin() + index);

			node->WiFiConnect(ssid, psk);
		}
		else if(input == "wifistartap") {
			printNodes(controller);

			int index = 0;
			std::string ssid, psk;

			std::cout << "Node Index: ";
			std::cin >> index;
			std::cout << "SSID: ";
			std::cin >> ssid;
			std::cout << "PSK: ";
			std::cin >> psk;

			//TODO: check bounds
			auto node = *(controller.nodeBegin() + index);

			node->WiFiStartAP(ssid, psk);
		}
		else {
			std::cout << "[Error] Unsupported operation '" << input << "'"
				<< std::endl;
		}
	}
	*/

	for(;;) {
		char msg[1024];

		std::cout << ">";
		std::cin.getline(msg, sizeof(msg));

		textEffect->setText(msg);
	}

	return 0;
}

void slotNodeDiscover(std::shared_ptr<LightNode> node) {
	node->addListener(LightNode::ListenerType::STATE_CHANGE, slotNodeStateChange);

	if(node->getName() == "bulb") {
		std::cout << "[Info] Bulb discovered" << std::endl;

		std::for_each(node->stripBegin(), node->stripEnd(),
			[](std::shared_ptr<LightStrip>& strip) {
				if(strip->getType() == LightStrip::Type::Analog) {
					bulbEffect->addStrip(strip);
				}
			});
	}
	else {
		std::cout << "[Info] New node discovered \"" << node->getName() << "\":" << std::endl;

		unsigned int analogCount = 0;
		std::for_each(node->stripBegin(), node->stripEnd(),
			[&analogCount](std::shared_ptr<LightStrip>& strip) {
				switch(strip->getType()) {
					case LightStrip::Type::Analog:
						if(analogCount == 0)
							rightEffect->addStrip(strip);
						else if(analogCount == 1)
							centerEffect->addStrip(strip);
						else if(analogCount < 4)
							leftEffect->addStrip(strip);
						else
							bulbEffect->addStrip(strip);
						
						analogCount++;

						std::cout << "\tAnalog strip (" << analogCount << ")" << std::endl;
					break;

					case LightStrip::Type::Digital:
						digitalEffect->addStrip(strip);
						std::cout << "\tDigital strip (" << strip->getSize() << ")" << std::endl;
					break;

					case LightStrip::Type::Matrix:
						if(strip->getSize() > 768) {
							visualizerEffect->addStrip(strip);
						}
						else {
							matrixEffect->addStrip(strip);
						}
						std::cout << "\tMatrix strip" << std::endl;
					break;

					default:
						std::cout << "\tUnknown strip (" << strip->getSize() << ")" << std::endl;
					break;
				}
			});
	}
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
