#include "main.hpp"

#define IP_ADDR	"192.168.1.3"
#define HUB_TO_NODE_PORT	54923
#define NODE_TO_HUB_PORT	54924

void slotNodeDiscover(std::shared_ptr<LightNode>);

void slotNodeStateChange(LightNode*, LightNode::State_e, LightNode::State_e);

int main() {
	//LightHub lightHub(HUB_TO_NODE_PORT, NODE_TO_HUB_PORT, LightHub::BROADCAST);

	//lightHub.addListener(LightHub::NODE_DISCOVER, &slotNodeDiscover);

	Rhopalia controller;

	controller.addListener(Rhopalia::NODE_DISCOVER, &slotNodeDiscover);

	for(;;) {
		//Delay 1 second
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	return 0;
}

void slotNodeDiscover(std::shared_ptr<LightNode> node) {
	std::cout << "[Info] slotNodeDiscover: New node discovered: '"
		<< node->getName() << "'" << std::endl;

	node->addListener(LightNode::STATE_CHANGE, &slotNodeStateChange);
}

void slotNodeStateChange(LightNode* node, LightNode::State_e prevState,
	LightNode::State_e newState) {

	std::cout << "[Info] slotNodeStateChange: Node '" << node->getName()
		<< "' state changed to " << LightNode::stateToString(newState)
		<< std::endl;
}
