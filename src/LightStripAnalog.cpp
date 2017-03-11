#include "LightStripAnalog.hpp"


LightBufferAnalog::LightBufferAnalog(LightStripAnalog *strip)
	:	LightBuffer(strip) {
}

Color LightBufferAnalog::getColor() const {
	return getPixelBufferConst().at(0);
}

void LightBufferAnalog::setColor(const Color& c) {
	getPixelBuffer().at(0) = c;
}


LightStripAnalog::LightStripAnalog()
	:	LightStrip(Type::Analog, 1) {
}

std::unique_ptr<LightBuffer> LightStripAnalog::getBuffer() {
	return std::make_unique<LightBufferAnalog>(this);
}
