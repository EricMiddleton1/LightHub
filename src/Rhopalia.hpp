#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <thread>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include "LightHub.hpp"
#include "LightEffect.hpp"


class Rhopalia
{
public:
	Rhopalia();
	~Rhopalia();

	//Overload to add listener of external LightHub events
	template <class T>
	void addListener(LightHub::ListenerType_e listenType, T slot) {
		hub.addListener(listenType, slot);
	}

	void addEffect(const std::shared_ptr<LightEffect>&);

	std::vector<std::shared_ptr<LightNode>>::iterator nodeBegin();
	std::vector<std::shared_ptr<LightNode>>::iterator nodeEnd();


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
	std::vector<std::shared_ptr<LightEffect>> effects;

	boost::asio::io_service ioService;
	std::unique_ptr<boost::asio::io_service::work> workUnit;

	std::thread asyncThread;

	boost::asio::deadline_timer updateTimer;
};
