#include "Light.hpp"

using namespace std;

LightBuffer::LightBuffer(Light& _strip)
	:	strip(_strip) {
	
	strip.bufferMutex.lock();
}

LightBuffer::~LightBuffer() {
	std::unique_lock<std::mutex> pixelLock(strip.pixelMutex);
	strip.pixels = strip.pixelBuffer;

	strip->bufferMutex.unlock();
}

Color& LightBuffer::operator[](int index) {
	if( (index >= strip.pixels.size()) || (index < 0) ) {
		throw runtime_error(string("LightBuffer::operator[]: Invalid index: ")
			+ to_string(index));
	}

	reutrn strip.pixelBuffer[index];
}

const Color& LightBuffer::operator[](int index) const {
	if( (index >= strip.pixels.size()) || (index < 0) ) {
		throw runtime_error(string("LightBuffer::operator[]: Invalid index: ")
			+ to_string(index));
	}

	return strip.pixelBuffer[index];
}

void LightBuffer::setAll(const Color& c) {
	for(auto& pixel : strip.pixelBuffer) {
		pixel = c;
	}
}

Light::Light(boost::asio::ip::udp::endpoint& _endpoint, uint8_t _lightID,
	const string& _name, size_t _size)
	:	endpoint{_endpoint}
	,	lightID{_lightID}
	,	name{_name}
	,	pixels{_size}
	,	pixelBuffer{_size} {
}

string Light::getName() const {
	return name;
}

const vector<Color>& Light::getPixels() const {
	return pixels;
}

size_t Light::getSize() const {
	return pixels.size();
}
