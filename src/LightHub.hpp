#pragma once

#include <vector> //std::vector
#include <memory> //std::shared_ptr
#include <thread> //std::thread
#include <iostream> //For debugging

#include <boost/asio.hpp> //networking, io_service
#include <boost/bind.hpp> //boost::bind

#include "LightNode.hpp"


class LightHub
{
public:
	enum ScanMethod_e {
		SCAN_SWEEP,
		SCAN_BROADCAST
	};

	//Exception codes
	static const uint16_t LIGHT_HUB_NODE_NOT_FOUND = 1;

	LightHub(uint16_t sendPort, uint16_t recvPort);
	~LightHub();

	void onNodeDiscover(std::function<void(std::shared_ptr<LightNode>)>);

	void scan(ScanMethod_e);

	std::shared_ptr<LightNode> getNodeByName(const std::string&);

	std::shared_ptr<LightNode> getNodeByAddress(const boost::asio::ip::address&);

	std::vector<std::shared_ptr<LightNode>>::iterator begin();
	std::vector<std::shared_ptr<LightNode>>::iterator end();

	size_t getNodeCount() const;

	size_t getConnectedNodeCount() const;

private:
	//Thread to run io_service.run()
	void threadRoutine();

	//Starts an async read on the UDP socket
	void startListening();

	//Callbacks for network operations
	void handleSendBroadcast(const boost::system::error_code&,
		size_t bytesTransferred);

	void handleReceive(const boost::system::error_code&,
		size_t bytesTransferred);

	//Callback for node state chance
	void cbNodeStateChange(LightNode*, LightNode::State_e, LightNode::State_e);

	//External callbacks
	std::function<void(std::shared_ptr<LightNode>)> extCbNodeDiscover;

	//Vector of nodes
	//That have at one point responded
	std::vector<std::shared_ptr<LightNode>> nodes;

	//Thread stuff
	boost::asio::io_service ioService;
	std::unique_ptr<boost::asio::io_service::work> ioWork;
	std::thread asyncThread;

	//Network stuff
	boost::asio::ip::udp::socket sendSocket, recvSocket;
	boost::asio::ip::udp::endpoint receiveEndpoint;
	uint16_t sendPort, recvPort;
	std::array<uint8_t, 512> readBuffer;


};
