#include "LightStripMatrix.hpp"

#include "font5x7.h"

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
			"LightBufferMatrix::setColor: x or y value too large ("
			+ std::to_string(x) + ", " + std::to_string(y) + ")");
	}

	getPixelBuffer()[y*matrix->width + x] = c;
}

void LightBufferMatrix::drawText(size_t x, size_t y, size_t offset, const Color& c,
	const std::string& str) {
	const size_t CHAR_WIDTH = 5;
	const size_t CHAR_HEIGHT = 7;

	size_t width = getWidth(), height = getHeight();
	
	size_t xPos = x, yPos = y;
	size_t strOffset = offset/(CHAR_WIDTH+1),
		chrOffset = offset % (CHAR_WIDTH+1);

	for(size_t i = strOffset; i < str.length(); ++i) {
		size_t fontIndex = CHAR_WIDTH*(str[i] - 32);
		
		for(size_t j = (i==strOffset)*chrOffset; j < CHAR_WIDTH && xPos < width; ++j) {
			unsigned char line = Font5x7[fontIndex + j];

			for(unsigned short pix = 0; pix < CHAR_HEIGHT; ++pix) {
				if(line & (1 << pix)) {
					setColor(xPos, yPos+pix, c);
				}
			}
			xPos++;
		}
		xPos++;
	}
}

size_t LightBufferMatrix::getWidth() const {
	LightStripMatrix *matrix = dynamic_cast<LightStripMatrix*>(strip);

	return matrix->width;
}

size_t LightBufferMatrix::getHeight() const {
	LightStripMatrix *matrix = dynamic_cast<LightStripMatrix*>(strip);

	return matrix->height;
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
