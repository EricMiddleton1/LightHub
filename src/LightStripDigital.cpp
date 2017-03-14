#include "LightStripDigital.hpp"


LightBufferDigital::LightBufferDigital(LightStripDigital *strip)
	:	LightBuffer(strip) {
}

LightBufferDigital LightBufferDigital::operator<<(size_t shamt) {
	auto &pixels = getPixelBuffer();

	Color last(pixels[pixels.size()-1]);

	for(size_t i = (pixels.size()-1); i > 0; --i) {
		pixels[i] = pixels[i-1];
	}

	//Circular shift
	pixels[0] = last;

	return *this;
}

LightBufferDigital LightBufferDigital::operator>>(size_t shamt) {
	auto &pixels = getPixelBuffer();

	Color first(pixels[0]);

	for(size_t i = (pixels.size()-1); i > 0; --i) {
		pixels[i-1] = pixels[i];
	}

	pixels[pixels.size()-1] = first;

	return *this;
}

size_t LightBufferDigital::getSize() const {
	return getPixelBufferConst().size();
}

Color LightBufferDigital::getColor(size_t i) const {
	return getPixelBufferConst().at(i);
}

void LightBufferDigital::setColor(size_t i, const Color& c) {
	getPixelBuffer().at(i) = c;
}


LightStripDigital::LightStripDigital(size_t _size)
	:	LightStrip(Type::Digital, _size) {
}

std::unique_ptr<LightBuffer> LightStripDigital::getBuffer() {
	return std::make_unique<LightBufferDigital>(this);
}
