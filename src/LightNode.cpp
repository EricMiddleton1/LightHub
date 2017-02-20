#include "LightNode.hpp"

using namespace std;

LightNode::LightNode(const string& _name, Type _type, uint16_t _ledCount,
	const boost::asio::ip::address& addr, uint16_t sendPort)
		:	name(_name)
		,	type{_type}
		,	width{(_type == Type::MATRIX) ? (_ledCount >> 8) : _ledCount}
		,	height{(_type == Type::MATRIX) ? (_ledCount & 0xFF) : 1}
		,	ledCount{width * height}
		,	strip{ledCount}
		,	isDirty{true}
		,	udpEndpoint(addr, sendPort)
		,	udpSocket(ioService)
		,	connectTimer(ioService)
		,	watchdogTimer(ioService)
		,	sendTimer(ioService)
		,	connectRetryCount{0}
		,	state{State::CONNECTING} {

	ioService.reset();

	//Create work unit so ioService doesn't return until deconstructor
	workPtr.reset(new boost::asio::io_service::work(ioService));

	udpSocket.open(boost::asio::ip::udp::v4());
	
	asyncThread = thread(std::bind(&LightNode::threadRoutine, this));

	sendPacket(Packet::Init());
	setConnectTimer();
}

LightNode::~LightNode() {
	//Make sure nobody has a reference to the LightStrip
	unique_lock<mutex> stripLock(stripMutex);

	//Clear the io_service work unit
	workPtr.reset();

	asyncThread.join();
}

void LightNode::addListener(ListenerType listenType,
	std::function<void(LightNode*, State, State)> slot) {

	//This is currently the only signal type
	if(listenType == ListenerType::STATE_CHANGE) {
		sigStateChange.connect(slot);
	}
	else {
		cout << "[Error] LightNode::addListener: Invalid listener type"
			<< endl;
	}
}

void LightNode::threadRoutine() {
	ioService.run();

	cout << "[Info] threadRoutine finished." << endl;
}

void LightNode::cbConnectTimer(const boost::system::error_code& error) {
	if(error.value() == boost::system::errc::operation_canceled) {
		return;
	}

	connectRetryCount++;
	if(connectRetryCount > PACKET_RETRY_COUNT) {
		changeState(State::DISCONNECTED);
	}
	else {
		sendPacket(Packet::Init());
		setConnectTimer();
	}
}

void LightNode::cbSendTimer(const boost::system::error_code& error) {
	if(error.value() == boost::system::errc::operation_canceled) {
		return;
	}
	
	if(state == State::CONNECTED) {
		//Send an alive packet so node doesn't disconnect
		sendPacket(Packet::Alive());
	}
}

void LightNode::cbWatchdogTimer(const boost::system::error_code& error) {
	if(error.value() == boost::system::errc::operation_canceled) {
		return;
	}

	//The watchdog timer expired, state is now DISCONNECTED
	changeState(State::DISCONNECTED);
}

void LightNode::cbSendPacket(uint8_t *buffer, const boost::system::error_code& error, size_t) {

	//TODO: deal with errors

	if(error) {
		cout << "[Error] LightNode::cbSendUpdate: " << error.message()
			<< endl;
	}

	delete[] buffer;
}

void LightNode::changeState(State newState) {
	if(state == newState) {
		return;
	}

	State oldState = state;
	state = newState;

	//If now CONNECTED, start watchdog and send timers
	if(newState == State::CONNECTED) {
		feedWatchdog();
		resetSendTimer();

		//Indicate that an update is needed
		isDirty = true;
	}
	else if(newState == State::CONNECTING) {
		connect();
	}

	//notify the callback
	sigStateChange(this, oldState, newState);
}

void LightNode::resetSendTimer() {
	sendTimer.cancel();

	sendTimer.expires_from_now(
		boost::posix_time::milliseconds(SEND_TIMEOUT));
	sendTimer.async_wait(std::bind(&LightNode::cbSendTimer, this, std::placeholders::_1));
}

void LightNode::feedWatchdog() {
	watchdogTimer.cancel();

	//Set watchdog timer
	watchdogTimer.expires_from_now(
		boost::posix_time::milliseconds(WATCHDOG_TIMEOUT));

	//Start watchdog timer
	watchdogTimer.async_wait(std::bind(&LightNode::cbWatchdogTimer, this, std::placeholders::_1));
}

void LightNode::setConnectTimer() {
	connectTimer.cancel();
	connectTimer.expires_from_now(boost::posix_time::milliseconds(CONNECT_TIMEOUT));
	connectTimer.async_wait(std::bind(&LightNode::cbConnectTimer, this, std::placeholders::_1));
}

void LightNode::connect() {
	if(state == State::CONNECTED)
		return;

	connectRetryCount = 0;

	sendPacket(Packet::Init());

	changeState(State::CONNECTING);
	setConnectTimer();
}

void LightNode::sendPacket(const Packet& p) {
	auto datagram = p.asDatagram();
	size_t bufferLen = datagram.size();
	uint8_t *buffer = new uint8_t[bufferLen];

	if(buffer == NULL)
		return;
	
	copy(datagram.begin(), datagram.end(), buffer);

	udpSocket.async_send_to(boost::asio::buffer(buffer, bufferLen), udpEndpoint,
		std::bind(&LightNode::cbSendPacket, this, buffer, std::placeholders::_1,
		std::placeholders::_2));

	if(state == State::CONNECTED) {
		resetSendTimer();
	}
}

void LightNode::disconnect() {
	changeState(State::DISCONNECTED);
}

LightNode::State LightNode::getState() const {
	return state;
}

string LightNode::getName() const {
	return name;
}

LightNode::Type LightNode::getType() const {
	return type;
}

boost::asio::ip::address LightNode::getAddress() const {
	return udpEndpoint.address();
}

void LightNode::receivePacket(const Packet& p) {
	switch(p.getID()) {
		case Packet::INFO:
			if(state == State::DISCONNECTED) {

				changeState(State::CONNECTING);
			}
		break;

		case Packet::ALIVE:
		case Packet::ACK:
			if(state == State::CONNECTING) {
				connectTimer.cancel();
			}

			if(state != State::CONNECTED) {
				changeState(State::CONNECTED);

			}
			else {
				feedWatchdog();
			}
		break;

		case Packet::NACK:
			if(p.getPayload()[0] == static_cast<unsigned char>(Packet::UPDATE)) {
				//NACKing an update is a fatal error
				disconnect();
			}
			else {
				cout << "[Warning] LightNode::receivePacket: Received NACK (" << strip.getSize() << ")" << endl;
			}
		break;

		default:
			cout << "[Warning] LightNode::receivePacket: Unexpected packet type (" << p.getID() << ")" << endl;
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
}

uint16_t LightNode::getWidth() const {
	return width;
}

uint16_t LightNode::getHeight() const {
	return height;
}

bool LightNode::update() {
	//Make sure the node is connected AND needs to be updated
	if(state != State::CONNECTED || !isDirty)
		return false;

	Packet p;

	{
		unique_lock<mutex> stripLock(stripMutex);

		p = Packet::Update(strip.getPixels());
	}

	sendPacket(p);

	//Clear the dirty bit
	isDirty = false;

	return true;
}

string LightNode::stateToString(State state) {
	string str;

	switch(state) {
		case State::DISCONNECTED:
			str = "disconnected";
		break;

		case State::CONNECTING:
			str = "connecting";
		break;

		case State::CONNECTED:
			str = "connected";
		break;

		default:
			str = "unknown";
		break;
	}

	return str;
}
