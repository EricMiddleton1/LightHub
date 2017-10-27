#include "Packet.hpp"

#include <stdexcept>

using namespace std;

Packet::Packet(ID _id)
	:	id{_id}
	,	lightID{0} {
}

Packet::Packet(ID _id, uint8_t _lightID)
	:	id{_id}
	,	lightID{_lightID} {
}

Packet::Packet(const std::vector<uint8_t>& data) {
	if(data.size() < 2) {
		throw runtime_error("Packet::Packet: vector is empty");
	}
	
	lightID = data[0];
	id = static_cast<ID>(data[1]);

	if(data.size() > 2) {
		payload = {data.begin()+2, data.end()};
	}
}

Packet Packet::NodeInfo() {
	return {ID::NodeInfo};
}

Packet Packet::NodeInfoResponse(uint8_t lightCount, const std::string& name) {
	Packet p(ID::NodeInfoResponse);
	p.payload.push_back(lightCount);
	p.payload.insert(p.payload.end(), name.begin(), name.end());

	return p;
}

Packet Packet::LightInfo(uint8_t lightID) {
	return {ID::LightInfo, lightID};
}

Packet Packet::LightInfoResponse(uint16_t ledCount, const std::string& name) {
	Packet p(ID::LightInfoResponse);
	
	auto countVec = pack16(ledCount);
	p.payload.insert(p.payload.end(), countVec.begin(), countVec.end());
	p.payload.insert(p.payload.end(), name.begin(), name.end());

	return p;
}

Packet Packet::UpdateColor(uint8_t lightID, const std::vector<Color>& leds) {
	Packet p{ID::UpdateColor, lightID};
	p.payload.push_back(0x07); //Update H, S, V

	for(const auto& led : leds) {
		p.payload.push_back(led.getHue());
		p.payload.push_back(led.getSat());
		p.payload.push_back(led.getVal());
	}

	return p;
}

Packet::ID Packet::getID() const {
	return id;
}

uint8_t Packet::getLightID() const {
	return lightID;
}

vector<uint8_t> Packet::data() const {
	return payload;
}

vector<uint8_t>::iterator Packet::begin() {
	return payload.begin();
}

vector<uint8_t>::iterator Packet::end() {
	return payload.end();
}

vector<uint8_t> Packet::asDatagram() const {
	vector<uint8_t> datagram;
	datagram.reserve(2 + payload.size());
	
	datagram.push_back(lightID);
	datagram.push_back(static_cast<uint8_t>(id));
	datagram.insert(datagram.end(), payload.begin(), payload.end());

	return datagram;
}

uint16_t Packet::parse16(vector<uint8_t>::const_iterator itr) {
	return ( (itr[0]) << 8 ) | itr[1];
}

array<uint8_t, 2> Packet::pack16(uint16_t value) {
	return {(value >> 8) & 0xFF, value & 0xFF};
}
