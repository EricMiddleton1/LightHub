#include "Packet.hpp"

Packet::Packet() {
	id = PING;
}

Packet::Packet(ID_e id, const std::vector<uint8_t>& payload) {
	this->id = id;
	this->payload = payload;
}

Packet::Packet(const std::vector<uint8_t>& datagram) {
	if(datagram.size() < 3) {
		throw Exception(PACKET_INVALID_SIZE,
			"[Packet::Packet]Error: Packet size < 3");
	}

	uint16_t header = datagram[0] << 8 | datagram[1];

	if(header != HEADER_VALUE) {
		throw Exception(PACKET_INVALID_HEADER,
			"[Packet::Packet]Error: Header value incorrect");
	}

	id = (ID_e)datagram[2];

	if(id == INFO) {
		if(datagram.size() < 7) {
			throw Exception(PACKET_INVALID_SIZE,
				"[Packet::Packet]Error: Packet size < 7 for type INFO");
		}
		payload = std::vector<uint8_t>(datagram.begin() + 3,
																					datagram.end());
	}
	else if(id == UPDATE) {
		if( ((datagram.size() - 3) % 3) != 0) { //check if divisible by 3
			throw Exception(PACKET_INVALID_SIZE,
				"[Packet::Packet]Error: Payload size not divisible by"
				"3 for type UPDATE");
		}
		payload = std::vector<uint8_t>(datagram.begin() + 3,
																					datagram.end());
	}
	else if(id == NACK) {
		if(datagram.size() != 4) {
			throw Exception(PACKET_INVALID_SIZE,
				"[Packet::Packet]Error: Packet size != 3 for type NACK");
		}
		payload = std::vector<uint8_t>({datagram[3]});
	}
	//All others have payload size = 0
}

std::vector<uint8_t> Packet::getPayload() const {
	return payload;
}

void Packet::setPayload(const std::vector<uint8_t>& payload) {
	this->payload = payload;
}

Packet::ID_e Packet::getID() const {
	return id;
}

void Packet::setID(Packet::ID_e id) {
	this->id = id;
}

std::vector<uint8_t> Packet::asDatagram() const{
	std::vector<uint8_t> datagram;

	//Two byte header value
	datagram.push_back(HEADER_VALUE >> 8);
	datagram.push_back(HEADER_VALUE & 0xFF);

	//One byte packet ID
	datagram.push_back((uint8_t)id);

	//payload
	for(const auto& b : payload) {
		datagram.push_back(b);
	}

	return datagram;
}

Packet Packet::Ping() {
	return Packet(PING, std::vector<uint8_t>());
}

Packet Packet::Init() {
	return Packet(INIT, std::vector<uint8_t>());
}

Packet Packet::Update(const std::vector<std::shared_ptr<LightStrip>>& strips) {
	std::vector<uint8_t> payload;
	
	for(const auto& strip : strips) {
		for(const auto& c : strip->getPixels()) {
			payload.push_back(c.getRed());
			payload.push_back(c.getGreen());
			payload.push_back(c.getBlue());
		}
	}

	return Packet(UPDATE, payload);
}

Packet Packet::Alive() {
	return Packet(ALIVE, std::vector<uint8_t>());
}

Packet Packet::Ack() {
	return Packet(ACK, std::vector<uint8_t>());
}

Packet Packet::Nack() {
	return Packet(NACK, std::vector<uint8_t>());
}

Packet Packet::WiFiConnect(const std::string& ssid, const std::string& psk) {
	std::vector<uint8_t> payload;

	for(auto& c : ssid) {
		payload.push_back(c);
	}
	payload.push_back(0);

	for(auto& c : psk) {
		payload.push_back(c);
	}
	payload.push_back(0);
	
	return Packet(WIFI_CONNECT, payload);
}

Packet Packet::WiFiStartAP(const std::string& ssid, const std::string& psk) {
	std::vector<uint8_t> payload;

	for(auto& c : ssid) {
		payload.push_back(c);
	}
	payload.push_back(0);

	for(auto& c : psk) {
		payload.push_back(c);
	}
	payload.push_back(0);
	
	return Packet(WIFI_START_AP, payload);
}
