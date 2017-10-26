#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <array>

class Packet {
public:
	enum class ID {
		NodeInfo = 0,
		NodeInfoResponse,
		LightInfo,
		LightInfoResponse,
		TurnOn,
		TurnOff,
		UpdateColor,
		ChangeBrightness
	};

	Packet(ID);
	Packet(const std::vector<uint8_t>& data);

	static Packet NodeInfoResponse(uint8_t lightCount, const std::string& name);
	static Packet LightInfoResponse(uint16_t ledCount, const std::string& name);

	ID getID() const;
	uint8_t getLightID() const;
	std::vector<uint8_t> data() const;
	
	std::vector<uint8_t>::iterator begin();
	std::vector<uint8_t>::iterator end();

private:
	static uint16_t parse16(std::vector<uint8_t>::const_iterator);
	static std::array<uint8_t, 2> pack16(uint16_t);

	ID id;
	uint8_t lightID;
	std::vector<uint8_t> payload;
};
