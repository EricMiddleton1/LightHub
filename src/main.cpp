#include "main.hpp"

#define IP_ADDR	"192.168.1.3"
#define HUB_TO_NODE_PORT	54923
#define NODE_TO_HUB_PORT	54924

int main() {
	LightHub lightHub(HUB_TO_NODE_PORT, NODE_TO_HUB_PORT, LightHub::BROADCAST);

	lightHub.onNodeDiscover([](std::shared_ptr<LightNode> node) {
		auto strip = node->getLightStrip();

		for(int i = 0; i < strip->getSize(); i++) {
			strip->setPixel(i, Color::HSV(i%360, 1, 1));
			node->sendUpdate();

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	});

	for(;;) {
		//Delay 1 second
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	return 0;
}
