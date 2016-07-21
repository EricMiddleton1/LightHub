#include "main.hpp"

#define IP_ADDR	"192.168.1.3"
#define HUB_TO_NODE_PORT	54923
#define NODE_TO_HUB_PORT	54924

void slotNodeDiscover(std::shared_ptr<LightNode>);

void slotNodeStateChange(LightNode*, LightNode::State_e, LightNode::State_e);

std::shared_ptr<LightEffectSolid> effectSolid;

int main() {
	Rhopalia controller;

	effectSolid = std::make_shared<LightEffectSolid>();

	effectSolid->setColor(Color(0, 255, 0));

	controller.addListener(LightHub::NODE_DISCOVER, &slotNodeDiscover);

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

	effectSolid->addNode(node);

	node->addListener(LightNode::STATE_CHANGE, &slotNodeStateChange);
}

void slotNodeStateChange(LightNode* node, LightNode::State_e prevState,
	LightNode::State_e newState) {

	std::cout << "[Info] slotNodeStateChange: Node '" << node->getName()
		<< "' state changed to " << LightNode::stateToString(newState)
		<< std::endl;
}
