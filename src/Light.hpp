#pragma once

#include <vector>
#include <string>
#include <mutex>
#include <memory>
#include <iostream>
#include <cstdint>

#include <boost/asio.hpp>

#include "Color.hpp"

class LightHub;
class LightBuffer;
struct LightNode;

class Light
{
public:
	Light(LightHub&, LightNode&, const boost::asio::ip::address& address, uint8_t lightID,
		const std::string& name, int size);

	boost::asio::ip::address getAddress() const;

	uint8_t getLightID() const;
	
	std::string getName() const;
	std::string getFullName() const;
	
	size_t getSize() const;
	const std::vector<Color>& getPixels() const;

	LightBuffer getBuffer();

	//TCP update commands
	void turnOn();
	void turnOff();
	void setBrightness(uint8_t brightness);
	void changeBrightness(int8_t deltaBrightness);
	void setColor(const Color& c);

private:
	friend class LightBuffer;

	void update();

	LightHub& hub;
	LightNode& node;

	boost::asio::ip::address address;
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

	int getSize() const;
	
	Color& operator[](int index);
	const Color& operator[](int index) const;

	void setAll(const Color& c);

private:
	Light& light;
};
