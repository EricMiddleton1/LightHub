#include "Packet.hpp"

#include <stdexcept>

using namespace std;

Packet::Packet(ID _id)
	:	id{_id} {
}

Packet::Packet(const std::vector<uint8_t>& data) {
	if(data.size() < 2) {
		throw runtime_error("Packet::Packet: vector is empty");
	}

	id = static_cast<ID>(data[0]);
	lightID = data[1];

	if(data.size() > 2) {
		payload = {data.begin()+2, data.end()};
	}
}

Packet Packet::NodeInfoResponse(uint8_t lightCount, const std::string& name) {
	Packet p(ID::NodeInfoResponse);
	p.payload.push_back(lightCount);
	p.payload.insert(p.payload.end(), name.begin(), name.end());

	return p;
}

Packet Packet::LightInfoResponse(uint16_t ledCount, const std::string& name) {
	Packet p(ID::LightInfoResponse);
	
	auto countVec = pack16(ledCount);
	p.payload.insert(p.payload.end(), countVec.begin(), countVec.end());
	p.payload.insert(p.payload.end(), name.begin(), name.end());

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

uint16_t Packet::parse16(vector<uint8_t>::const_iterator itr) {
	return ( (itr[0]) << 8 ) | itr[1];
}

array<uint8_t, 2> Packet::pack16(uint16_t value) {
	return {(value >> 8) & 0xFF, value & 0xFF};
}
