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
	using PluginHandler = std::function<void(std::shared_ptr<Plugin>)>;

	enum ListenerType_e {
		CONNECT,
		DISCONNECT
	};


	PluginServer(uint16_t port);
	~PluginServer();
	
	template<class EventHandler>
	void addListener(ListenerType_e listenType, EventHandler slot) {
		switch(listenType) {
			case CONNECT:
				sigConnect.connect(slot);
			break;

			case DISCONNECT:
				sigDisconnect.connect(slot);
			break;
		}
	}

private:
	//Async thread routine
	void threadRoutine();

	
	//Connected plugins
	std::vector<Plugin> effects;

	//Signals
	boost::signals2::signal<PluginHandler> sigConnect, sigDisconnect;

	//General boost stuff
	boost::asio::io_service ioService;
	std::unique_ptr<boost::asio::io_service::work> ioWork;

	//Network stuff
	boost::asio::ip::tcp::socket listenSocket;

	//Thread stuff
	std::thread asyncThread;
};
