#include "LightHub.hpp"


LightHub::LightHub(uint16_t _port)
	:	udpSocket(ioService, boost::asio::ip::udp::endpoint(
			boost::asio::ip::udp::v4(), _port)) {
	port = _port;

	//Construct the work unit for the io_service
	ioWork.reset(new boost::asio::io_service::work(ioService));

	//Start the async thread
	asyncThread = std::thread(std::bind(&LightHub::threadRoutine, this));

	//Start listening for packets
	startListening();
}

LightHub::~LightHub() {
	//Delete the work unit, allow io_service.run() to return
	ioWork.reset();

	//Wait for the async thread to complete
	asyncThread.join();
}

void LightHub::threadRoutine() {
	//Run all async tasks
	ioService.run();

	std::cout << "[Info] LightHub: Thread closing" << std::endl;
}


void LightHub::scan(LightHub::ScanMethod_e method) {

	switch(method) {
		case SCAN_SWEEP:
			std::cout << "[Error] LightHub::Scan method 'SCAN_SWEEP' "
				<< "not implemented" << std::endl;
		break;

		case SCAN_BROADCAST:
			//Send ping broadcast packet
			udpSocket.async_send_to(boost::asio::buffer(
				Packet::Ping().asDatagram()),
				boost::asio::ip::address_v4::broadcast(),
				boost::bind(&LightHub::handleSendBroadcast, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		break;
	}
}


void LightHub::handleSendBroadcast(const boost::system::error_code& ec,
	size_t bytesTransferred) {
	if(ec) {
		std::cout << "[Error] Failed to send broadcast ping message"
			<< std::endl;
	}
}

void LightHub::startListening() {
	//Start the async receive
	udpSocket.async_receive_from(boost::asio::buffer(readBuffer),
		receiveEndpoint,
		boost::bind(&LightHub::handleReceive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void LightHub::handleReceive(const boost::system::error_code& ec,
	size_t bytesTransferred) {
	
	if(ec) {
		std::cout << "[Error] Failed to receive from UDP socket" << std::endl;
	}
	else {
		//Parse the datagram into a packet
		Packet p(std::vector<uint8_t>(std::beg(readBuffer),
			std::beg(readBuffer) + bytesTransferred);

		//try to find the node associated with this packet
		auto node = getNodeByAddress(receiveEndpoint.address());

		if(node) {
			//Let the node handle the packet
			node->receivePacket(p);
		}
		else {
			//The sender is not in the list of connected nodes
			//Create a new node in the list
			nodes.emplace_back("", receiveEndpoint.address(), port, 
}
