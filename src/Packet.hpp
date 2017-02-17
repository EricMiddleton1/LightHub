#pragma once

#include <cstdint> //for int typedefs
#include <vector> //for std::vector

#include "Color.hpp"
#include "Exception.hpp"

class Packet
{
public:
	static const int PACKET_INVALID_HEADER = 0x0010;
	static const int PACKET_INVALID_SIZE = 0x0011;

	enum ID_e {
		PING = 0x00,
		INIT = 0x01,
		INFO = 0x02,
		UPDATE = 0x03,
		ALIVE = 0x04,
		
		ACK = 0xFE,
		NACK = 0xFF
	};


	Packet();

	Packet(ID_e id, const std::vector<uint8_t>& payload);
	
	Packet(const std::vector<uint8_t>& datagram);


	std::vector<uint8_t> getPayload() const;
	void setPayload(const std::vector<uint8_t>& payload);

	ID_e getID() const;
	void setID(ID_e id);

	std::vector<uint8_t> asDatagram() const;
	
	static Packet Ping();
	static Packet Init();
	static Packet Info(uint16_t pixelCount);
	static Packet Update(const std::vector<Color>& colors);
	static Packet Alive();
	static Packet Ack();
	static Packet Nack();

private:
	const static uint16_t HEADER_VALUE = 0xAA55;

	ID_e id;
	std::vector<uint8_t> payload;

};
