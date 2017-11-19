#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <array>

#include "Color.hpp"

class Packet {
public:
	enum class ID {
		NodeInfo = 0,
		NodeInfoResponse,
		LightInfo,
		LightInfoResponse,
		TurnOn,
		TurnOff,
		SetBrightness,
		SetColor,
		ChangeBrightness,
		UpdateColor
	};

	Packet(ID);
	Packet(ID id, uint8_t lightID);

	Packet(const std::vector<uint8_t>& data);
	
	static Packet NodeInfo();
	static Packet NodeInfoResponse(uint8_t lightCount, const std::string& name);
	static Packet LightInfo(uint8_t lightID);
	static Packet LightInfoResponse(uint16_t ledCount, const std::string& name);
	static Packet UpdateColor(uint8_t lightID, uint8_t hPeriod, uint8_t sPeriod,
		uint8_t vPeriod, const std::vector<Color>& leds);

	ID getID() const;
	uint8_t getLightID() const;
	std::vector<uint8_t> data() const;
	
	std::vector<uint8_t>::iterator begin();
	std::vector<uint8_t>::iterator end();

	std::vector<uint8_t> asDatagram() const;

	static uint16_t parse16(std::vector<uint8_t>::const_iterator);
	static std::array<uint8_t, 2> pack16(uint16_t);

private:
	ID id;
	uint8_t lightID;
	std::vector<uint8_t> payload;
};
