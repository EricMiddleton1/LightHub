#include "Light.hpp"

#include "LightHub.hpp"

using namespace std;

LightBuffer::LightBuffer(Light& _light)
	:	light{_light} {
	
	light.bufferMutex.lock();
}

LightBuffer::~LightBuffer() {
	{
		lock_guard<mutex> pixelLock(light.pixelMutex);
		light.pixels = light.pixelBuffer;
		light.transitionPeriod = light.transitionPeriodBuffer;

		light.bufferMutex.unlock();
	}

	light.update();
}

int LightBuffer::getSize() const {
	return light.pixelBuffer.size();
}

Color& LightBuffer::operator[](int index) {
	if( (index >= light.pixels.size()) || (index < 0) ) {
		throw runtime_error(string("LightBuffer::operator[]: Invalid index: ")
			+ to_string(index));
	}

	return light.pixelBuffer[index];
}

const Color& LightBuffer::operator[](int index) const {
	if( (index >= light.pixels.size()) || (index < 0) ) {
		throw runtime_error(string("LightBuffer::operator[]: Invalid index: ")
			+ to_string(index));
	}

	return light.pixelBuffer[index];
}

void LightBuffer::setAll(const Color& c) {
	for(auto& pixel : light.pixelBuffer) {
		pixel = c;
	}
}

void LightBuffer::setTransitionPeriod(uint8_t period) {
	light.transitionPeriodBuffer = period;
}

uint8_t LightBuffer::getTransitionPeriod() const {
	return light.transitionPeriodBuffer;
}

Light::Light(LightHub& _hub, LightNode& _node, const boost::asio::ip::address& _address,
	uint8_t _lightID, const string& _name, int _size)
	:	hub{_hub}
	,	node{_node}
	,	address{_address}
	,	lightID{_lightID}
	,	name{_name}
	,	transitionPeriod{0}
	,	transitionPeriodBuffer{0}
	,	pixels{_size}
	,	pixelBuffer{_size} {
}

boost::asio::ip::address Light::getAddress() const {
	return address;
}

uint8_t Light::getLightID() const {
	return lightID;
}

string Light::getName() const {
	return name;
}

string Light::getFullName() const {
	return node.name + "/" + name;
}

const vector<Color>& Light::getPixels() const {
	std::lock_guard<std::mutex> pixelLock{pixelMutex};

	return pixels;
}

uint8_t Light::getTransitionPeriod() const {
	std::lock_guard<std::mutex> pixelLock{pixelMutex};

	return transitionPeriod;
}

size_t Light::getSize() const {
	return pixels.size();
}

void Light::update() {
	hub.update(*this);
}

LightBuffer Light::getBuffer() {
	return {*this};
}
