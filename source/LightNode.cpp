#include "LightNode.hpp"


LightNode::LightNode(const std::string& name,
	const boost::asio::ip::address& addr, int port)
		:	udpEndpoint(addr, port)
		,	udpSocket(ioService) {

	//Create work unit so ioService doesn't return until deconstructor
	workPtr.reset(new boost::asio::io_service::work(ioService));

	this->name = name;

	//TODO: send status request
	//TODO: start watchdog timers

	//Launch async thread
	asyncThread.reset(new std::thread(std::bind(&LightNode::threadRoutine,
		this)));
}

LightNode::~LightNode() {
	//Delete work unit and stop ioService
	workPtr.reset();
	ioService.stop();

	//Wait for the async thread to finish
	asyncThread.join();
}

void LightNode::threadRoutine() {
	//Continually process ioService tasks
	ioService.run();

	std::cout << "[" << CLASS_NAME << "] threadRoutine finished." << std::endl;
}
