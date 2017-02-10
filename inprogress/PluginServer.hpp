#pragma once

#include <stdint.h>
#include <thread>
#include <vector>
#include <functional>
#include <memory>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

class PluginServer
{
public:
	using EventHandler = std::function<void(std::shared_ptr<Effect>)>;
	using ServiceHandler = std::function<void(std::shared_ptr<Service>)>;

	enum ListenerType_e {
		EFFECT_CONNECT,
		EFFECT_DISCONNECT,

		SERVICE_CONNECT,
		SERVICE_DISCONNECT
	};


	PluginServer(uint16_t port);
	~PluginServer();
	
	std::vector<std::shared_ptr>::iterator effectBegin();
	std::vector<std::shared_ptr>::iterator effectEnd();

	std::vector<std::shared_ptr>::iterator serviceBegin();
	std::vector<std::shared_ptr>::iterator serviceEnd();

	size_t getEffectCount() const;
	size_t getServiceCount() const;

	
	template<class EventHandler>
	void addListener(ListenerType_e listenType, EventHandler slot) {
		switch(listenType) {
			case EFFECT_CONNECT:
				sigEffectConnect.connect(slot);
			break;

			case EFFECT_DISCONNECT:
				sigEffectDisconnect.connect(slot);
			break;

			case SERVICE_CONNECT:
				sigServiceConnect.connect(slot);
			break;

			case SERVICE_DISCONNECT:
				sigServiceDisconnect.connect(slot);
		}
	}

private:
	//Async thread routine
	void threadRoutine();

	
	//Connected plugins
	std::vector<Effect> effects;
	std::vector<Service> services;

	//Signals
	boost::signals2::signal<EventHandler> sigEffectConnect, sigEffectDisconnect;
	boost::signals2::signal<ServiceHandler> sigServiceConnect, sigServiceDisconnect;

	//General boost stuff
	boost::asio::io_service ioService;
	std::unique_ptr<boost::asio::io_service::work> ioWork;

	//Network stuff
	boost::asio::ip::tcp::socket listenSocket;

	//Thread stuff
	std::thread asyncThread;

	
	


};
