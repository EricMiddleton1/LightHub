#include "LightNode.hpp"


LightNode::LightNode(const std::string& _name,
	const boost::asio::ip::address& addr, uint16_t sendPort)
		:	name(_name)
		,	pixelCount{0}
		,	isDirty{false}
		,	udpEndpoint(addr, sendPort)
		,	udpSocket(ioService)
		,	infoTimer(ioService)
		,	watchdogTimer(ioService) 
		,	infoRetryCount{0}
		,	state{CONNECTING} {

	ioService.reset();

	//Create work unit so ioService doesn't return until deconstructor
	workPtr.reset(new boost::asio::io_service::work(ioService));

	//Open the socket
	try {
		udpSocket.open(boost::asio::ip::udp::v4());
	}
	catch(const std::exception& e) {
		std::cout << "LightNode::LightNode: Exception caught on socket open: "
			<< e.what() << std::endl;
	}

	//Send status request
	sendInfoRequest();

	//Launch async thread
	asyncThread = std::thread(std::bind(&LightNode::threadRoutine, this));
}

LightNode::~LightNode() {
	//Make sure nobody has a reference to the LightStrip
	std::unique_lock<std::mutex> stripLock(stripMutex);

	//Clear the io_service work unit
	workPtr.reset();

	//Wait for the async thread to finish
	asyncThread.join();
}

void LightNode::addListener(ListenerType_e listenType,
	std::function<void(LightNode*, State_e, State_e)> slot) {

	//This is currently the only signal type
	if(listenType == STATE_CHANGE) {
		sigStateChange.connect(slot);
	}
	else {
		//This shouldn't happen
		std::cout << "[Error] LightNode::addListener: Invalid listener type"
			<< std::endl;
	}
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

	if(error) {
		std::cout << "[Error] LightNode::cbInfo: " << error.message()
			<< std::endl;
	}
}

void LightNode::cbSendUpdate(const boost::system::error_code& error,
	size_t bytesTransferred) {

	//TODO: deal with errors

	if(error) {
		std::cout << "[Error] LightNode::cbSendUpdate: " << error.message()
			<< std::endl;
	}
}

void LightNode::changeState(State_e newState) {
	if(state == newState) {
		//This is not a state change
		return;
	}

	State_e oldState = state;
	state = newState;

	//If DISCONNECTED->CONNECTING, request status from node
	if(oldState == DISCONNECTED && newState == CONNECTING)
		sendInfoRequest();

	//If now CONNECTED, start watchdog timer
	if(newState == CONNECTED) {
		feedWatchdog();

		//Indicate that an update is needed
		isDirty = true;

		//Send a blank update
		//sendUpdate();
		//DON'T do this anymore, all updates done in sync
	}

	//notify the callback
	sigStateChange(this, oldState, newState);
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

std::string LightNode::getName() const {
	return name;
}

boost::asio::ip::address LightNode::getAddress() const {
	return udpEndpoint.address();
}

void LightNode::receivePacket(Packet& p) {
//	std::cout << "[Info] LightNode::receivePacket: Packet received with ID "
//		<< p.getID() << std::endl;

	switch(p.getID()) {
		case Packet::INFO: {
			int _pixelCount = (p.getPayload()[0] << 8) | p.getPayload()[1];

			if(_pixelCount != pixelCount) {
				std::unique_lock<std::mutex> stripLock(stripMutex);

				pixelCount = _pixelCount;
				strip = LightStrip(pixelCount);
			}

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

//TODO: Return a class that manages the mutex state in RAII-style
LightStrip& LightNode::getLightStrip() {
	//Lock the strip mutex
	stripMutex.lock();

	return strip;
}

void LightNode::releaseLightStrip(bool _isDirty) {
	stripMutex.unlock();

	isDirty = _isDirty;

	if(isDirty)
		std::cout << "[Info] LightNode::releaseLightStrip: LightNode now dirty"
			<< std::endl;
}

bool LightNode::update() {
	//Make sure the node is connected AND needs to be updated
	if(state != CONNECTED || !isDirty)
		return false;

	std::vector<uint8_t> datagram;

	{
		//Lock the strip mutex
		std::unique_lock<std::mutex> stripLock(stripMutex);

		datagram = Packet::Update(strip.getPixels()).asDatagram();
	} //Mutex gets unlocked here

	std::cout << "[Info] LightNode::update: Sending update to '" << name
		<< "' with color " << strip.getPixels()[0].toString() << std::endl;

	udpSocket.async_send_to(boost::asio::buffer(datagram), udpEndpoint,
		[this](const boost::system::error_code& error,
			size_t bytesTransferred) {
				cbSendUpdate(error, bytesTransferred);
		});

	//Clear the dirty bit
	isDirty = false;

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
