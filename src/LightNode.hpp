#pragma once

#include <string> //std::string
#include <thread> //std::thread
#include <functional> //std::bind, std::function
#include <memory> //std::shared_ptr
#include <iostream> //DEBUG, for std::cout, std::endl
#include <mutex>
#include <cstdint>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/signals2.hpp>

#include "LightStrip.hpp"
#include "Packet.hpp"
#include "PeriodicTimer.hpp"
#include "WatchdogTimer.hpp"


//Forward declaration of friend class
class LightHub;

class LightNode
{
public:
	enum class State {
		DISCONNECTED = 0,
		CONNECTING,
		CONNECTED
	};

	enum class ListenerType {
		STATE_CHANGE
	};

	LightNode(boost::asio::io_service& ioService, const std::string& name,
		const std::vector<std::shared_ptr<LightStrip>>&,
		const boost::asio::ip::address& addr, uint16_t sendPort);
	
	void addListener(ListenerType,
		std::function<void(LightNode*, State, State)>);

	void connect();
	void disconnect();

	void WiFiConnect(const std::string& ssid, const std::string& psk);
	void WiFiStartAP(const std::string& ssid, const std::string& psk);

	State getState() const;

	std::string getName() const;

	std::vector<std::shared_ptr<LightStrip>>::iterator stripBegin();
	std::vector<std::shared_ptr<LightStrip>>::iterator stripEnd();

	boost::asio::ip::address getAddress() const;
	
	static std::string stateToString(State state);

private:
	const int CONNECT_TIMEOUT = 1000;
	const int SEND_TIMEOUT = 1000;
	const int RECV_TIMEOUT = 3000;
	const int PACKET_RETRY_COUNT = 3;

	friend class LightHub;

	bool update();

	void receivePacket(const Packet& p);

	void threadRoutine();

	void sendPacket(const Packet& p);

	void connectTimerHandler();
	void sendTimerHandler();
	void recvTimerHandler();

	void cbSendPacket(uint8_t *buffer, const boost::system::error_code& error,
		size_t bytesTransferred);

	void changeState(State newState);

	//Remote node information
	std::string name;
	std::vector<std::shared_ptr<LightStrip>> strips;

	//Signals
	boost::signals2::signal<void(LightNode*, State, State)> sigStateChange;

	//Network stuff
	boost::asio::io_service& ioService;
	boost::asio::ip::udp::endpoint udpEndpoint;
	boost::asio::ip::udp::socket udpSocket;

	//Timer stuff
	std::unique_ptr<PeriodicTimer> connectTimer;
	WatchdogTimer recvTimer;
	WatchdogTimer sendTimer;

	int connectRetryCount;

	State state;
};
