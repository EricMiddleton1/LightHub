#include "LightNode.hpp"


LightNode::LightNode(const std::string& name,
	const boost::asio::ip::address& addr, int port,
	const std::function<void(State_e, State_e)>& cbStateChange)
		:	udpEndpoint(addr, port)
		,	udpSocket(ioService)
		,	statusTimer(ioService)
		,	watchdogTimer(ioService) {


	ioService.reset();

	//Create work unit so ioService doesn't return until deconstructor
	workPtr.reset(new boost::asio::io_service::work(ioService));

	this->name = name;
	this->cbStateChange = cbStateChange;

	statusRetryCount = 0;

	state = CONNECTING;

	//Send status request
	sendStatusRequest();

	//Set watchdog timer
	//watchdogTimer.expires_from_now(
		//boost::posix_time::milliseconds(WATCHDOG_TIMEOUT));

	//Start watchdog timer
	//watchdogTimer.async_wait([this](const boost::system::error_code& error) {
		///cbWatchdogTimer(error);
	//});

	//Launch async thread
	asyncThread.reset(new std::thread(std::bind(&LightNode::threadRoutine,
		this)));
}

LightNode::~LightNode() {
	//Clear the io_service work unit
	workPtr.reset();

	//Wait for the async thread to finish
	asyncThread->join();
}

void LightNode::threadRoutine() {
	ioService.run();

	std::cout << "[LightNode] threadRoutine finished." << std::endl;
}

void LightNode::sendStatusRequest() {
	//Send status request packet
	udpSocket.async_send_to(boost::asio::buffer(Packet::Init().asDatagram()),
		udpEndpoint,
		[this](const boost::system::error_code& error, size_t bytesTransferred) {
			cbStatus(error, bytesTransferred);
		});

	//Set status request timeout timer
	statusTimer.expires_from_now(
		boost::posix_time::milliseconds(PACKET_TIMEOUT));

	//Start timeout timer
	statusTimer.async_wait([this](const boost::system::error_code& error) {
		cbStatusTimer(error);
	});
}

void LightNode::cbStatusTimer(const boost::system::error_code& error) {
	if(error.value() == boost::system::errc::operation_canceled) {
		//We manually cancelled the timer
		return;
	}

	if(statusRetryCount >= PACKET_RETRY_COUNT) {
		//We've already made enough attemps, we're disconnected
		changeState(DISCONNECTED);

		return;
	}

	std::cout << "[LightNode::cbStatusTimer] Retrying status request"
		<< std::endl;

	statusRetryCount++;

	sendStatusRequest();
}

void LightNode::cbWatchdogTimer(const boost::system::error_code& error) {
	if(error.value() == boost::system::errc::operation_canceled) {
		//We manually reset the watchdog timer
		return;
	}

	//The watchdog timer expired, state is now DISCONNECTED
	changeState(DISCONNECTED);
}

void LightNode::cbStatus(const boost::system::error_code& error,
	size_t bytesTransferred) {

	//TODO: deal with errors
}

void LightNode::cbSendUpdate(const boost::system::error_code& error,
	size_t bytesTransferred) {

	//TODO: deal with errors
}

void LightNode::changeState(State_e newState) {
	if(state == newState) {
		//this is not a state change
		return;
	}

	State_e oldState = state;
	state = newState;

	//If DISCONNECTED->CONNECTING, request status from node
	if(oldState == DISCONNECTED && newState == CONNECTING)
		sendStatusRequest();

	//If now CONNECTED, start watchdog timer
	if(newState == CONNECTED)
		feedWatchdog();

	//notify the callback
	cbStateChange(oldState, newState);
}

void LightNode::feedWatchdog() {
	watchdogTimer.cancel();

	//Set watchdog timer
	watchdogTimer.expires_from_now(
		boost::posix_time::milliseconds(WATCHDOG_TIMEOUT));

	//Start watchdog timer
	watchdogTimer.async_wait([this](const boost::system::error_code& error) {
		cbWatchdogTimer(error);
	});

}

void LightNode::receivePacket(Packet& p) {
	switch(p.getID()) {
		case Packet::INFO: {
			int pixelCount = (p.getPayload()[0] << 8) | p.getPayload()[1];

			if(pixelCount != strip.getSize())
				strip = LightStrip(pixelCount);

			//Cancel the timeout timer
			statusTimer.cancel();

			changeState(CONNECTED);
		}
		break;

		case Packet::ALIVE:
		case Packet::ACK:
			//Feed the watchdog
			feedWatchdog();

			if(state == DISCONNECTED) {
				changeState(CONNECTING);
			}
		break;

		default:
		//Do something
		break;
	}
}

LightStrip& LightNode::getLightStrip() {
	return strip;
}

bool LightNode::sendUpdate() {
	if(state != CONNECTED)
		return false;
	
	auto datagram = Packet::Update(strip.getPixels()).asDatagram();

	udpSocket.async_send_to(boost::asio::buffer(datagram), udpEndpoint,
		[this](const boost::system::error_code& error,
			size_t bytesTransferred) {
				cbSendUpdate(error, bytesTransferred);
		});

	return true;
}
