#include "main.hpp"

#define IP_ADDR	"192.168.1.3"
#define HUB_TO_NODE_PORT	54923
#define NODE_TO_HUB_PORT	54924

void slotNodeDiscover(std::shared_ptr<LightNode>);

void slotNodeStateChange(LightNode*, LightNode::State_e, LightNode::State_e);

std::shared_ptr<LightEffectSoundSolid> effect;

int main() {


	//Add the effect to the controller
	//controller.addEffect(effectFade);
	
	//Create an audio device
	std::shared_ptr<AudioDevice> audioDevice =
		std::make_shared<AudioDevice>(AudioDevice::DEFAULT_DEVICE, 48000, 1024);

	//Create a spectrum analyzer
	std::shared_ptr<SpectrumAnalyzer> spectrumAnalyzer =
		std::make_shared<SpectrumAnalyzer>(audioDevice,32.7032,16744.0384,3, 1);
	
	//Configure SoundColor settings
	SoundColorSettings scs;
	scs.bassFreq = 120.;
	scs.trebbleFreq = 4000.;
	scs.bassBoost = 0.;
	scs.trebbleBoost = 20.;
	scs.fStart = 0;
	scs.fEnd = 20000;
	scs.dbScaler = 60.;
	scs.dbFactor = 0.6;
	scs.avgFactor = 0.5;
	scs.noiseFloor = 150.;
	scs.slopeLimitAvg = 0.2;
	scs.minSaturation = 0.5;
	scs.filterStrength = 0.2;
	scs.centerSpread = 0.5;
	scs.centerBehavior = SoundColorSettings::MONO;

	//Create a SoundColor
	//SoundColor soundColor(spectrumAnalyzer, scs);

	effect = std::make_shared<LightEffectSoundSolid>(spectrumAnalyzer, scs);
	


	Rhopalia controller;

	controller.addListener(LightHub::NODE_DISCOVER, &slotNodeDiscover);

	controller.addEffect(effect);
	//Start the audio device
	audioDevice->startStream();





	//Everything is handled by other threads now
	for(;;) {
		//Delay 1 second
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	return 0;
}

void slotNodeDiscover(std::shared_ptr<LightNode> node) {
	std::cout << "[Info] slotNodeDiscover: New node discovered: '"
		<< node->getName() << "'" << std::endl;

	effect->addNode(node);

	node->addListener(LightNode::STATE_CHANGE, &slotNodeStateChange);
}

void slotNodeStateChange(LightNode* node, LightNode::State_e,
	LightNode::State_e newState) {

	std::cout << "[Info] slotNodeStateChange: Node '" << node->getName()
		<< "' state changed to " << LightNode::stateToString(newState)
		<< std::endl;
}
