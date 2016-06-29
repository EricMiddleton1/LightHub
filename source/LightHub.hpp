#include <vector> //std::vector
#include <memory> //std::shared_ptr
#include <thread> //std::thread
#include <iostream> //For debugging

#include <boost/asio.hpp> //networking, io_service

#include "LightNode.hpp"


class LightHub
{
public:
	enum ScanMethod_e {
		SCAN_SWEEP,
		SCAN_BROADCAST
	};

	LightHub(uint16_t port);
	~LightHub();

	void scan(ScanMethod_e);

	std::shared_ptr<LightNode> getNodeByName(const std::string&);
	std::shared_ptr<LightNode> getNodeByAddress(const boost::ip::address&);


	

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


	//Vector of nodes
	//That have at one point responded
	std::vector<std::shared_ptr<LightNode>> nodes;

	//Thread stuff
	boost::asio::io_service ioService;
	std::unique_ptr<boost::asio::io_service::work> ioWork;
	std::thread asyncThread;

	//Network stuff
	boost::asio::ip::udp::socket udpSocket;
	boost::asio::ip::udp::endpoint receiveEndpoint;
	uint16_t port;
	std::array<uint8_t, 512> readBuffer;


}
