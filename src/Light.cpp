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
		light.transitionPeriods = light.transitionPeriodsBuffer;

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

Color& LightBuffer::at(int index) {
	if( (index >= light.pixels.size()) || (index < 0) ) {
		throw runtime_error(string("LightBuffer::operator[]: Invalid index: ")
			+ to_string(index));
	}

	return light.pixelBuffer[index];
}

const Color& LightBuffer::at(int index) const {
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

void LightBuffer::setTransitionPeriod(
	const std::tuple<uint8_t, uint8_t, uint8_t>& periods) {
	
	light.transitionPeriodsBuffer = periods;
}

Light::Light(LightHub& _hub, LightNode& _node, const boost::asio::ip::address& _address,
	uint8_t _lightID, const string& _name, int _size)
	:	hub{_hub}
	,	node{_node}
	,	address{_address}
	,	lightID{_lightID}
	,	name{_name}
	,	transitionPeriods{0, 0, 0}
	,	transitionPeriodsBuffer{0, 0, 0}
	,	pixels{_size}
	,	pixelBuffer{_size} {
}

Light::~Light() {
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

uint8_t Light::getHuePeriod() const {
	return std::get<0>(transitionPeriods);
}

uint8_t Light::getSatPeriod() const {
	return std::get<1>(transitionPeriods);
}

uint8_t Light::getValPeriod() const {
	return std::get<2>(transitionPeriods);
}

size_t Light::getSize() const {
	return pixels.size();
}

void Light::update() {
	hub.update(*this);
}

std::unique_ptr<LightBuffer> Light::getBuffer() {
	return std::make_unique<LightBuffer>(*this);
}
