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


//Forward declaration of friend class
class LightHub;

class LightNode
{
public:
	enum Type_e {
		ANALOG = 0,
		DIGITAL,
		MATRIX
	};

	enum State_e {
		DISCONNECTED = 0,
		CONNECTING,
		CONNECTED
	};

	enum ListenerType_e {
		STATE_CHANGE
	};

	LightNode(const std::string& name, Type_e type, uint16_t ledCount, 
		const boost::asio::ip::address& addr, uint16_t sendPort);
	
	~LightNode();

	void addListener(ListenerType_e,
		std::function<void(LightNode*, State_e, State_e)>);

	void connect();
	void disconnect();

	State_e getState() const;

	std::string getName() const;
	Type_e getType() const;

	boost::asio::ip::address getAddress() const;

	LightStrip& getLightStrip();
	void releaseLightStrip(bool isDirty = true);

	static std::string stateToString(State_e state);

private:
	static const int CONNECT_TIMEOUT = 1000;
	static const int SEND_TIMEOUT = 1000;
	static const int WATCHDOG_TIMEOUT = 3000;
	static const int PACKET_RETRY_COUNT = 3;

	friend class LightHub;

	bool update();

	void receivePacket(const Packet& p);

	void threadRoutine();

	void sendPacket(const Packet& p);

	void cbConnectTimer(const boost::system::error_code& error);
	void cbSendTimer(const boost::system::error_code& error);
	void cbWatchdogTimer(const boost::system::error_code& error);

	void cbSendPacket(uint8_t *buffer, const boost::system::error_code& error,
		size_t bytesTransferred);

	void changeState(State_e newState);

	void resetSendTimer();
	void feedWatchdog();
	void setConnectTimer();

	//Remote strip information
	std::string name;
	Type_e type;
	LightStrip strip;
	uint16_t pixelCount;
	std::mutex stripMutex;
	bool isDirty; //Indicates that the node needs to be updated

	//Signals
	boost::signals2::signal<void(LightNode*, State_e, State_e)> sigStateChange;

	//Network stuff
	boost::asio::io_service ioService;
	boost::asio::ip::udp::endpoint udpEndpoint;
	boost::asio::ip::udp::socket udpSocket;

	//Timer stuff
	boost::asio::deadline_timer connectTimer;
	boost::asio::deadline_timer watchdogTimer;
	boost::asio::deadline_timer sendTimer;

	int connectRetryCount;

	//Thread stuff
	std::unique_ptr<boost::asio::io_service::work> workPtr;
	std::thread asyncThread;

	State_e state;
};
