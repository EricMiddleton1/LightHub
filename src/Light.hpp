#pragma once

#include <vector>
#include <string>
#include <mutex>
#include <memory>
#include <tuple>
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
	virtual ~Light();

	boost::asio::ip::address getAddress() const;

	uint8_t getLightID() const;
	
	std::string getName() const;
	std::string getFullName() const;
	
	size_t getSize() const;
	const std::vector<Color>& getPixels() const;
	uint8_t getHuePeriod() const;
	uint8_t getSatPeriod() const;
	uint8_t getValPeriod() const;

	virtual std::unique_ptr<LightBuffer> getBuffer();

protected:
	friend class LightBuffer;

	void update();

	LightHub& hub;
	LightNode& node;

	boost::asio::ip::address address;
	uint8_t lightID;

	std::string name;
	
	std::tuple<uint8_t, uint8_t, uint8_t> transitionPeriods, transitionPeriodsBuffer;
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

	Color& at(int index);
	const Color& at(int index) const;

	void setAll(const Color& c);

	void setTransitionPeriod(const std::tuple<uint8_t, uint8_t, uint8_t>& periods);

protected:
	Light& light;
};
