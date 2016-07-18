#pragma once

#include <vector>
#include <memory>

#include "LightHub.hpp"
#include "ILightEffect.hpp"


class Rhopalia
{
public:
	enum ListenerType_e {
		NODE_DISCOVER
	};

	Rhopalia();
	~Rhopalia();

	template <class T>
	void addListener(ListenerType_e listenType, T slot) {
		if(listenType == NODE_DISCOVER) {
			hub.addListener(LightHub::NODE_DISCOVER, slot);
		}
		else {
			std::cout << "[Error] Rhopalia::addListener: Invalid listener type"
				<< std::endl;
		}
	}


private:
	//TODO: Read these values from a config file
	static const uint16_t SEND_PORT = 54923;
	static const uint16_t RECV_PORT = 54924;
	static const LightHub::DiscoveryMethod_e DISCOVER_METHOD =
		LightHub::BROADCAST;

	LightHub hub;
	std::vector<ILightEffect> lightEffects;
};
