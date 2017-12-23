#pragma once

#include <vector>
#include <memory>
#include <thread>
#include <iostream>
#include <deque>
#include <map>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include "Light.hpp"
#include "PeriodicTimer.hpp"


class Rhopalia;

struct LightNode {
	LightNode(const std::string& name);

	std::string name;
	std::vector<std::shared_ptr<Light>> lights;
	std::vector<int> timeouts;
};


class LightHub
{
public:
	using NodeIterator = std::map<boost::asio::ip::address, LightNode>::const_iterator;

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

	NodeIterator begin() const;
	NodeIterator end() const;

	size_t getNodeCount() const;

private:
	static const int LIGHT_TIMEOUT;

	friend class Rhopalia;
	friend class Light;

	void update(Light& light);

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
	void handleDiscoveryTimer();

	//Signals
	boost::signals2::signal<void(std::shared_ptr<Light>)> sigLightDiscover;

	std::map<boost::asio::ip::address, LightNode> nodes;

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
