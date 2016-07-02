#include "main.hpp"

#define IP_ADDR	"192.168.1.3"
#define HUB_TO_NODE_PORT	54923
#define NODE_TO_HUB_PORT	54924

int main() {
	/*
	LightNode testNode("TestNode",
		boost::asio::ip::address::from_string(IP_ADDR),
		PORT, [](LightNode::State_e oldState, LightNode::State_e newState) {
			std::cout << "[Info] State changed from " <<
				LightNode::stateToString(oldState) << " to " <<
				LightNode::stateToString(newState) << std::endl;
		});

	std::cout << "[Info] testNode state: " <<
		LightNode::stateToString(testNode.getState()) << std::endl;
	*/

	LightHub lightHub(HUB_TO_NODE_PORT, NODE_TO_HUB_PORT);

	lightHub.onNodeDiscover([](std::shared_ptr<LightNode> node) {
		auto strip = node->getLightStrip();

		for(int i = 0; i < strip->getSize(); i++) {
			strip->setPixel(i, Color::HSV(i%360, 1, 1));
			node->sendUpdate();

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	});

	lightHub.scan(LightHub::SCAN_BROADCAST);

	for(;;) {
		//Do a broadcast scan for nodes
//		std::cout << "[Info] Scanning for new nodes" << std::endl;
		lightHub.scan(LightHub::SCAN_BROADCAST);

		//Delay 1 second
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	return 0;
}
