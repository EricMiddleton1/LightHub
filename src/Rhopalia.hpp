#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <thread>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include "LightHub.hpp"
#include "ILightEffect.hpp"


class Rhopalia
{
public:
	enum ListenerType_e {
		REFRESH_TIMER
	};

	Rhopalia();
	~Rhopalia();

	//Overload to add listener of internal Rhopalia events
	template <class T>
	void addListener(ListenerType_e listenType, T slot) {
		if(listenType == REFRESH_TIMER) {
			sigRefreshTimer.connect(slot);
		}
		else {
			std::cout << "[Error] Rhopalia::addListener: Invalid listener type"
				<< std::endl;
		}
	}

	//Overload to add listener of external LightHub events
	template <class T>
	void addListener(LightHub::ListenerType_e listenType, T slot) {
		hub.addListener(listenType, slot);
	}


private:
	//TODO: Read these values from a config file
	static const uint16_t SEND_PORT = 54923;
	static const uint16_t RECV_PORT = 54924;
	static const LightHub::DiscoveryMethod_e DISCOVER_METHOD =
		LightHub::BROADCAST;
	
	static const uint32_t UPDATE_PERIOD = 20; //Gives update rate ~= 50fps

	void threadRoutine();

	void startUpdateTimer();
	void cbUpdateTimer(const boost::system::error_code&);

	LightHub hub;
	std::vector<ILightEffect> lightEffects;

	boost::asio::io_service ioService;
	std::unique_ptr<boost::asio::io_service::work> workUnit;

	std::thread asyncThread;

	boost::asio::deadline_timer updateTimer;

	//Signals
	boost::signals2::signal<void()> sigRefreshTimer;
};
