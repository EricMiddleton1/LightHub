#ifndef LIGHTNODE_HPP
#define LIGHTNODE_HPP

#include <string> //std::string
#include <thread> //std::thread
#include <functional> //std::bind, std::function
#include <memory> //std::shared_ptr
#include <iostream> //DEBUG, for std::cout, std::endl

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "LightStrip.hpp"
#include "Packet.hpp"


class LightNode
{
public:
	enum State_e {
		DISCONNECTED,
		CONNECTING,
		CONNECTED
	};

	LightNode(const std::string& name,
		const boost::asio::ip::address& addr, int port,
		const std::function<void(State_e, State_e)>& cbStateChange);
	
	~LightNode();

	void connect();
	void disconnect();

	State_e getState() const;

	void receivePacket(Packet& p);

	LightStrip& getLightStrip();

	bool sendUpdate();

	static std::string stateToString(State_e state);

private:
	static const int PACKET_TIMEOUT = 1000;
	static const int WATCHDOG_TIMEOUT = 10000;
	static const int PACKET_RETRY_COUNT = 3;

	void threadRoutine();

	void cbInfoTimer(const boost::system::error_code& error);
	void cbWatchdogTimer(const boost::system::error_code& error);

	void cbInfo(const boost::system::error_code& error,
		size_t bytesTransferred);`
	void cbSendUpdate(const boost::system::error_code& error,
		size_t bytesTransferred);

	void sendInfoRequest();

	void changeState(State_e newState);

	void feedWatchdog();

	std::string name;
	LightStrip strip;

	boost::asio::io_service ioService;
	boost::asio::ip::udp::endpoint udpEndpoint;
	boost::asio::ip::udp::socket udpSocket;

	boost::asio::deadline_timer infoTimer;
	boost::asio::deadline_timer watchdogTimer;

	int infoRetryCount;

	std::function<void(State_e, State_e)> cbStateChange;

	std::unique_ptr<boost::asio::io_service::work> workPtr;
	std::thread asyncThread;

	State_e state;
};

#endif
