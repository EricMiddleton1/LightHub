#ifndef PACKET_HPP
#define PACKET_HPP

#include <cstdint> //for int typedefs
#include <array> //for std::array
#include <vector> //for std::vector

class Packet
{
public:
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

	Packet(ID_e id, const std::vector& payload);
	
	Packet(const std::array<uint8_t>& datagram);


	std::vector<uint8_t>& getPayload();
	void setPayload(const std::vector<uint8_t>& payload)

	ID_e getID();
	void setID(ID_e id);

	
	static Packet Ping();
	static Packet Init();
	static Packet Info(uint16_t pixelCount);
	static Packet Update(const std::vector<

private:
	ID_e id;
	std::vector<uint8_t> payload;

};


#endif
