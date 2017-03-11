#include "LightStripDigital.hpp"


LightBufferDigital::LightBufferDigital(LightStripDigital *strip)
	:	LightBuffer(strip) {
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
