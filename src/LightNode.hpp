#pragma once

#include <string> //std::string
#include <thread> //std::thread
#include <functional> //std::bind, std::function
#include <memory> //std::shared_ptr
#include <iostream> //DEBUG, for std::cout, std::endl
#include <mutex>

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
	enum State_e {
		DISCONNECTED,
		CONNECTING,
		CONNECTED
	};

	enum ListenerType_e {
		STATE_CHANGE
	};

	LightNode(const std::string& name,
		const boost::asio::ip::address& addr, uint16_t sendPort);
	
	~LightNode();

	void addListener(ListenerType_e,
		std::function<void(LightNode*, State_e, State_e)>);

	void connect();
	void disconnect();

	State_e getState() const;

	std::string getName() const;

	boost::asio::ip::address getAddress() const;

	void receivePacket(Packet& p);

	LightStrip& getLightStrip();
	void releaseLightStrip(bool isDirty = true);

	static std::string stateToString(State_e state);

private:
	static const int PACKET_TIMEOUT = 1000;
	static const int WATCHDOG_TIMEOUT = 10000;
	static const int PACKET_RETRY_COUNT = 3;

	friend class LightHub;

	//Function to update the remote LightNode
	bool update();

	void threadRoutine();

	void cbInfoTimer(const boost::system::error_code& error);
	void cbWatchdogTimer(const boost::system::error_code& error);

	void cbInfo(const boost::system::error_code& error,
		size_t bytesTransferred);
	void cbSendUpdate(const boost::system::error_code& error,
		size_t bytesTransferred);

	void sendInfoRequest();

	void changeState(State_e newState);

	void feedWatchdog();

	//Remote strip information
	std::string name;
	LightStrip strip;
	uint16_t pixelCount;
	std::mutex stripMutex; //Mutex to protect access to strip
	bool isDirty; //Indicates that the node needs to be updated

	//Signals
	boost::signals2::signal<void(LightNode*, State_e, State_e)> sigStateChange;

	//Network stuff
	boost::asio::io_service ioService;
	boost::asio::ip::udp::endpoint udpEndpoint;
	boost::asio::ip::udp::socket udpSocket;

	//Timer stuff
	boost::asio::deadline_timer infoTimer;
	boost::asio::deadline_timer watchdogTimer;

	int infoRetryCount;

	//Thread stuff
	std::unique_ptr<boost::asio::io_service::work> workPtr;
	std::thread asyncThread;

	State_e state;
};
