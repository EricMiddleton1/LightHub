#include "main.hpp"

#define IP_ADDR	"192.168.1.3"
#define PORT	30962

int main() {
	LightNode testNode("TestNode",
		boost::asio::ip::address::from_string(IP_ADDR),
		PORT, [](LightNode::State_e oldState, LightNode::State_e newState) {
			std::cout << "State changed from " << (int)oldState << " to "
				<< (int)newState << std::endl;
		});

	for(;;) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	return 0;
}
