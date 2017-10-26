#pragma once

#include <vector>
#include <string>
#include <mutex>
#include <memory>
#include <iostream>
#include <cstdint>

#include <boost/asio.hpp>

#include "Color.hpp"

class LightBuffer;

class Light
{
public:
	Light(const boost::asio::ip::udp::endpoint& endpoint, uint8_t lightID,
		const std::string& name, int size);

	boost::asio::ip::udp::endpoint getEndpoint() const;
	uint8_t getLightID() const;
	std::string getName() const;
	size_t getSize() const;
	const std::vector<Color>& getPixels() const;

	LightBuffer getBuffer();

private:
	friend class LightBuffer;

	boost::asio::ip::udp::endpoint endpoint;
	uint8_t lightID;

	std::string name;

	std::vector<Color> pixels, pixelBuffer;
	mutable std::mutex pixelMutex, bufferMutex;
};

class LightBuffer
{
public:
	LightBuffer(Light&);
	virtual ~LightBuffer();
	
	Color& operator[](int index);
	const Color& operator[](int index) const;

	void setAll(const Color& c);

private:
	Light& strip;
};
