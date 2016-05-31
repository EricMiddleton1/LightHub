#ifndef LIGHTNODE_HPP
#define LIGHTNODE_HPP

#include <string> //std::string
#include <thread> //std::thread
#include <functional> //std::bind
#include <memory> //std::shared_ptr
#include <iostream> //DEBUG, for std::cout, std::endl

#include <boost/asio.hpp>

#include "LightStrip.hpp"


class LightNode
{
public:
	enum State_e {
		DISCONNECTED,
		CONNECTED,
		INACTIVE
	};

	LightNode(const std::string& name,
		const boost::asio::ip::address& addr, int port);
	~LightNode();

	void receivePacket(const Packet& p);


private:
	const static char* CLASS_NAME = (char*)"LightNode";

	void threadRoutine();

	std::string name;
	LightStrip strip;

	boost::asio::io_service ioService;
	boost::asio::ip::udp::endpoint udpEndpoint;
	boost::asio::ip::udp::socket udpSocket;

	std::thread asyncThread;

	std::unique_ptr<boost::asio::io_service::work> workPtr;
	std::unique_ptr<std::thread> asyncThread;

	State_e;

};

#endif
