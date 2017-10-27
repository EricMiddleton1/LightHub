#pragma once

#include <vector>
#include <memory>
#include <functional>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include "LightHub.hpp"
#include "LightEffect.hpp"
#include "PeriodicTimer.hpp"


class Rhopalia
{
public:
	Rhopalia();
	~Rhopalia();
	
	void run();

	//Overload to add listener of external LightHub events
	template <class T>
	void addListener(LightHub::ListenerType listenType, T slot) {
		hub.addListener(listenType, slot);
	}

	void addEffect(const std::shared_ptr<LightEffect>&);

	LightHub::NodeIterator nodeBegin();
	LightHub::NodeIterator nodeEnd();

private:
	//TODO: Read these values from a config file
	uint16_t PORT = 5492;
	const uint32_t UPDATE_PERIOD = 20;

	void update();

	LightHub hub;
	std::vector<std::shared_ptr<LightEffect>> effects;

	boost::asio::io_service ioService;
	std::unique_ptr<boost::asio::io_service::work> workUnit;

	PeriodicTimer updateTimer;
};
