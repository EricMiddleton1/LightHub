#pragma once

#include <vector>
#include <memory>
#include <thread>
#include <iostream>
#include <deque>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include "Light.hpp"
#include "PeriodicTimer.hpp"


class Rhopalia;

class LightHub
{
public:
	enum class ListenerType {
		LightDiscover
	};

	LightHub(uint16_t port, uint32_t discoverPeriod = 1000);
	~LightHub();

	template<class T>
	void addListener(ListenerType listenType, T slot) {
		if(listenType == ListenerType::LightDiscover) {
			sigLightDiscover.connect(slot);
		}
		else {
			std::cout << "[Error] LightNode::addListener: Invalid listener type"
				<< std::endl;
		}
	}

	std::vector<std::shared_ptr<LightNode>>::iterator begin();
	std::vector<std::shared_ptr<LightNode>>::iterator end();

	size_t getNodeCount() const;

private:
	friend class Rhopalia;

	void threadRoutine();

	void startListening();

	void discover();

	void sendDatagram(const boost::asio::ip::address& addr,
		const std::vector<uint8_t>& data);

	void handleSendBroadcast(const boost::system::error_code&,
		size_t bytesTransferred);

	void handleReceive(const boost::system::error_code&,
		size_t bytesTransferred);

	//Callback for discovery timer
	void handleDiscoveryTimer(const boost::system::error_code&);

	//Signals
	boost::signals2::signal<void(std::shared_ptr<Light>)> sigLightDiscover;

	std::vector<std::shared_ptr<Light>> lights;
	std::unordered_map<boost::asio::ip::address, std::string> nodeMap;

	//Thread stuff
	boost::asio::io_service ioService;
	std::unique_ptr<boost::asio::io_service::work> ioWork;
	std::thread asyncThread;

	//Network stuff
	boost::asio::ip::udp::socket socket;
	boost::asio::ip::udp::endpoint receiveEndpoint;
	uint16_t port;
	std::array<uint8_t, 512> readBuffer;
	std::deque<std::vector<uint8_t>> sendQueue;
	mutable std::mutex sendMutex;

	//Autodiscovery stuff
	PeriodicTimer discoveryTimer;
};
