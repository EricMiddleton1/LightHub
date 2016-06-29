#include "LightNode.hpp"


LightNode::LightNode(const std::string& name,
	const boost::asio::ip::address& addr, int port,
	const std::function<void(State_e, State_e)>& cbStateChange)
		:	udpEndpoint(addr, port)
		,	udpSocket(ioService)
		,	infoTimer(ioService)
		,	watchdogTimer(ioService) {


	ioService.reset();

	//Create work unit so ioService doesn't return until deconstructor
	workPtr.reset(new boost::asio::io_service::work(ioService));

	this->name = name;
	this->cbStateChange = cbStateChange;

	infoRetryCount = 0;

	state = CONNECTING;

	//Send status request
	sendInfoRequest();

	//Launch async thread
	asyncThread = std::thread(std::bind(&LightNode::threadRoutine, this));
}

LightNode::~LightNode() {
	//Clear the io_service work unit
	workPtr.reset();

	//Wait for the async thread to finish
	asyncThread.join();
}

void LightNode::threadRoutine() {
	ioService.run();

	std::cout << "[Info] threadRoutine finished." << std::endl;
}

void LightNode::sendInfoRequest() {
	//Send status request packet
	udpSocket.async_send_to(boost::asio::buffer(Packet::Init().asDatagram()),
		udpEndpoint,
		[this](const boost::system::error_code& error, size_t bytesTransferred) {
			cbInfo(error, bytesTransferred);
		});

	//Set status request timeout timer
	infoTimer.expires_from_now(
		boost::posix_time::milliseconds(PACKET_TIMEOUT));

	//Start timeout timer
	infoTimer.async_wait([this](const boost::system::error_code& error) {
		cbInfoTimer(error);
	});
}

void LightNode::cbInfoTimer(const boost::system::error_code& error) {
	if(error.value() == boost::system::errc::operation_canceled) {
		//We manually cancelled the timer
		return;
	}

	if(infoRetryCount >= PACKET_RETRY_COUNT) {
		//We've already made enough attemps, we're disconnected
		changeState(DISCONNECTED);

		return;
	}

	std::cout << "[Info] Retrying info request"
		<< std::endl;

	infoRetryCount++;

	sendInfoRequest();
}

void LightNode::cbWatchdogTimer(const boost::system::error_code& error) {
	if(error.value() == boost::system::errc::operation_canceled) {
		//We manually reset the watchdog timer
		return;
	}

	//The watchdog timer expired, state is now DISCONNECTED
	changeState(DISCONNECTED);
}

void LightNode::cbInfo(const boost::system::error_code& error,
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
		sendInfoRequest();

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

void LightNode::connect() {
	if(state == DISCONNECTED)
		changeState(CONNECTING);
}

void LightNode::disconnect() {
	changeState(DISCONNECTED);
}

LightNode::State_e LightNode::getState() const {
	return state;
}

void LightNode::receivePacket(Packet& p) {
	switch(p.getID()) {
		case Packet::INFO: {
			int pixelCount = (p.getPayload()[0] << 8) | p.getPayload()[1];

			if(pixelCount != strip.getSize())
				strip = LightStrip(pixelCount);

			//Cancel the timeout timer
			infoTimer.cancel();

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

std::string LightNode::stateToString(State_e state) {
	std::string str;

	switch(state) {
		case DISCONNECTED:
			str = "disconnected";
		break;

		case CONNECTING:
			str = "connecting";
		break;

		case CONNECTED:
			str = "connected";
		break;

		default:
			str = "unknown";
		break;
	}

	return str;
}
