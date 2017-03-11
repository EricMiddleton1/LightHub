#include "LightStripMatrix.hpp"


LightBufferMatrix::LightBufferMatrix(LightStripMatrix *strip)
	:	LightBuffer(strip) {
}

Color LightBufferMatrix::getColor(size_t x, size_t y) const {
	LightStripMatrix *matrix = dynamic_cast<LightStripMatrix*>(strip);

	if(x >= matrix->width || y >= matrix->height) {
		throw Exception(LightStrip::INVALID_INDEX,
			"LightBufferMatrix::getColor: x or y value too large");
	}

	return getPixelBufferConst()[y*matrix->width + x];
}

void LightBufferMatrix::setColor(size_t x, size_t y, const Color& c) {
	LightStripMatrix *matrix = dynamic_cast<LightStripMatrix*>(strip);

	if(x >= matrix->width || y >= matrix->height) {
		throw Exception(LightStrip::INVALID_INDEX,
			"LightBufferMatrix::setColor: x or y value too large");
	}

	getPixelBuffer()[y*matrix->width + x] = c;
}


LightStripMatrix::LightStripMatrix(size_t _width, size_t _height)
	:	LightStrip(Type::Matrix, _width * _height)
	,	width{_width}
	,	height{_height} {
}

size_t LightStripMatrix::getWidth() const {
	return width;
}

size_t LightStripMatrix::getHeight() const {
	return height;
}

std::unique_ptr<LightBuffer> LightStripMatrix::getBuffer() {
	return std::make_unique<LightBufferMatrix>(this);
}
