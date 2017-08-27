#include "LightNode.hpp"

#include <iostream>
#include <chrono>

using namespace std;

LightNode::LightNode(boost::asio::io_service& _ioService, const string& _name,
	const std::vector<std::shared_ptr<LightStrip>>& _strips,
	const boost::asio::ip::udp::endpoint& _endpoint)
		:	name(_name)
		,	strips{_strips}
		, ioService{_ioService}
		,	udpEndpoint(_endpoint)
		,	udpSocket(ioService, boost::asio::ip::udp::v4())
		,	recvTimer(ioService, std::chrono::milliseconds(RECV_TIMEOUT),
			[this] () { recvTimerHandler(); })
		,	sendTimer(ioService, std::chrono::milliseconds(SEND_TIMEOUT),
			[this] () { sendTimerHandler(); })
		,	connectRetryCount{0}
		,	state{State::CONNECTING} {
	
	connect();
	startListening();
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

void LightNode::startListening() {
	udpSocket.async_receive_from(boost::asio::buffer(readBuffer),
		recvEndpoint, [this](const boost::system::error_code& ec, size_t bytesTransferred) {
			if(!ec && (recvEndpoint.address() == udpEndpoint.address())) {
				Packet p;

				try {
					p = Packet(std::vector<uint8_t>(std::begin(readBuffer),
						std::begin(readBuffer) + bytesTransferred));
				}
				catch(const Exception& e) {
					std::cout << "[Error] LightHub::handleReceive: Invalid datagram: " << e.what()
						<< std::endl;

					startListening();
					return;
				}

				receivePacket(p);
			}

			startListening();
		});
}

void LightNode::connectTimerHandler() {
	connectRetryCount++;

	if(connectRetryCount > PACKET_RETRY_COUNT) {
		changeState(State::DISCONNECTED);
		connectTimer.reset();
	}
	else {
		sendPacket(Packet::Init());
	}
}

void LightNode::sendTimerHandler() {
	if(state == State::CONNECTED) {
		//Send an alive packet so node doesn't disconnect
		sendPacket(Packet::Alive());
	}
}

void LightNode::recvTimerHandler() {
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
		recvTimer.start();
		sendTimer.start();
	}
	else if(newState == State::CONNECTING) {
		connect();
	}
	else {
		//Stop the watchdog timers
		recvTimer.stop();
		sendTimer.stop();
	}

	//notify the callback
	sigStateChange(this, oldState, newState);
}

void LightNode::connect() {
	if(state == State::CONNECTED)
		return;

	connectRetryCount = 0;

	sendPacket(Packet::Init());

	changeState(State::CONNECTING);

	connectTimer = std::make_unique<PeriodicTimer>(ioService,
		std::chrono::milliseconds(CONNECT_TIMEOUT), [this]() { std::cout << "[Info] ConnectTimer" << std::endl; connectTimerHandler(); });
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
		sendTimer.feed();
	}
}

void LightNode::disconnect() {
	changeState(State::DISCONNECTED);
}

void LightNode::WiFiConnect(const std::string& ssid, const std::string& psk) {
	sendPacket(Packet::WiFiConnect(ssid, psk));
}

void LightNode::WiFiStartAP(const std::string& ssid, const std::string& psk) {
	sendPacket(Packet::WiFiStartAP(ssid, psk));
}

LightNode::State LightNode::getState() const {
	return state;
}

string LightNode::getName() const {
	return name;
}

std::vector<std::shared_ptr<LightStrip>>::iterator LightNode::stripBegin() {
	return strips.begin();
}

std::vector<std::shared_ptr<LightStrip>>::iterator LightNode::stripEnd() {
	return strips.end();
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
				connectTimer.reset();
			}

			if(state != State::CONNECTED) {
				changeState(State::CONNECTED);

			}
			else {
				recvTimer.feed();
			}
		break;

		case Packet::NACK:
			if(p.getPayload().size() < 1) {
				std::cout << "[Error] LightNode::receivePacket: Received NACK with no payload"
					<< std::endl;
			}
			else if(p.getPayload()[0] == static_cast<unsigned char>(Packet::UPDATE)) {
				//NACKing an update is a fatal error
				disconnect();
			}
			else {
				cout << "[Warning] LightNode::receivePacket: Received NACK" << endl;
			}
		break;

		default:
			cout << "[Warning] LightNode::receivePacket: Unexpected packet type (" << p.getID() << ")" << endl;
		break;
	}
}

bool LightNode::update() {
	if(state != State::CONNECTED)
		return false;

	Packet p;

	p = Packet::Update(strips);

	sendPacket(p);

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
