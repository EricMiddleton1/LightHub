#include <thread>
#include <chrono>
#include <memory>
#include <iostream>

#include "Rhopalia.hpp"

#include "LightEffectSoundSolid.hpp"
#include "LightEffectSoundMove.hpp"
#include "LightEffectMatrixEQ.hpp"
#include "LightEffectStripEQ.hpp"
#include "LightEffectStripStEQ.hpp"

#define IP_ADDR	"192.168.1.3"
#define HUB_TO_NODE_PORT	54923
#define NODE_TO_HUB_PORT	54924

void slotNodeDiscover(std::shared_ptr<LightNode>);

void slotNodeStateChange(LightNode*, LightNode::State, LightNode::State);

std::shared_ptr<ILightEffect> analogEffect;
std::shared_ptr<ILightEffect> digitalEffect;
std::shared_ptr<ILightEffect> matrixEffect;

void printNodes(Rhopalia&);

int main() {
	//Create an audio device
	std::shared_ptr<AudioDevice> audioDevice =
		std::make_shared<AudioDevice>(AudioDevice::DEFAULT_DEVICE, 48000, 1024);

	//Create a spectrum analyzer
	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer =
		std::make_shared<SpectrumAnalyzer>(audioDevice,32.7032,16744.0384,3,4096,1);

	//Configure SoundColor settings
	SoundColorSettings scs;
	scs.bassFreq = 150.;
	scs.trebbleFreq = 4000.;
	scs.bassBoost = 15.;
	scs.trebbleBoost = 0.;
	scs.fStart = 0;
	scs.fEnd = 20000;
	scs.dbScaler = 200;
	scs.dbFactor = 1;
	scs.avgFactor = 0.2;
	scs.noiseFloor = 60.;
	scs.avgFilterStrength = 0.4;
	scs.minSaturation = 0.7;
	scs.filterStrength = 0.7;
	scs.centerSpread = 0.5;
	scs.centerBehavior = SoundColorSettings::MONO;

	//Create a SoundColor
	SoundColor soundColor(spectrumAnalyzer, scs);

	analogEffect = std::make_shared<LightEffectSoundSolid>(spectrumAnalyzer, scs);
	//digitalEffect = std::make_shared<LightEffectSoundMove>(spectrumAnalyzer);
	digitalEffect = std::make_shared<LightEffectStripStEQ>(spectrumAnalyzer);
	matrixEffect = std::make_shared<LightEffectMatrixEQ>(spectrumAnalyzer);

	Rhopalia controller;

	controller.addListener(LightHub::NODE_DISCOVER, &slotNodeDiscover);
	
	controller.addEffect(analogEffect);
	controller.addEffect(digitalEffect);
	controller.addEffect(matrixEffect);

	//Start the audio device
	audioDevice->startStream();

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

	return 0;
}

void slotNodeDiscover(std::shared_ptr<LightNode> node) {
	node->addListener(LightNode::ListenerType::STATE_CHANGE, slotNodeStateChange);

	size_t ledCount = node->getLightStrip().getSize();
	node->releaseLightStrip();

	std::cout << "[Info] slotNodeDiscover: New node discovered: '"
		<< node->getName() << "' of type '"<< static_cast<int>(node->getType())
		<< "' with " << ledCount << " leds" << std::endl;
	
	if(node->getType() == LightNode::Type::ANALOG) {
		analogEffect->addNode(node);
	}
	else if(node->getType() == LightNode::Type::DIGITAL) {
		digitalEffect->addNode(node);
	}
	else if(node->getType() == LightNode::Type::MATRIX) {
		matrixEffect->addNode(node);
	}
	else {
		std::cout << "[Error] Node connected with unknown type" << std::endl;
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
