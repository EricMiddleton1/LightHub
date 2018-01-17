#include "Matrix.hpp"

#include <stdexcept>

#include "font5x7.h"

Matrix::Matrix(LightHub& hub, LightNode& node, const boost::asio::ip::address& addr,
	uint8_t lightID, const std::string& name, uint8_t _width, uint8_t _height)
	:	Light(hub, node, addr, lightID, name, _width*_height)
	,	width{_width}
	,	height{_height} {
}

Matrix::~Matrix() {
}

uint8_t Matrix::getWidth() const {
	return width;
}

uint8_t Matrix::getHeight() const {
	return height;
}

std::unique_ptr<LightBuffer> Matrix::getBuffer() {
	return buffer_cast<LightBuffer, MatrixBuffer>(std::make_unique<MatrixBuffer>(*this));
}

bool Matrix::isMatrix(const std::shared_ptr<Light>& light) {
	auto matrix = std::dynamic_pointer_cast<Matrix>(light);

	return !!matrix;
}

MatrixBuffer::MatrixBuffer(Matrix& matrix)
	:	LightBuffer(dynamic_cast<Matrix&>(matrix)) {
}

uint8_t MatrixBuffer::getWidth() const {
	return dynamic_cast<Matrix&>(light).width;
}

uint8_t MatrixBuffer::getHeight() const {
	return dynamic_cast<Matrix&>(light).height;
}

Color MatrixBuffer::get(uint8_t x, uint8_t y) const {
	auto width = getWidth(), height = getHeight();

	if( (x >= width) || (y >= height) ) {
		throw std::invalid_argument("MatrixBuffer::get: Invalid coordinate: ("
			+ std::to_string(x) + ", " + std::to_string(y) + ")");
	}

	return at(y*width + x);
}

void MatrixBuffer::set(uint8_t x, uint8_t y, const Color& c) {
	auto width = getWidth(), height = getHeight();

	if( (x >= width) || (y >= height) ) {
		throw std::invalid_argument("MatrixBuffer::set: Invalid coordinate: ("
			+ std::to_string(x) + ", " + std::to_string(y) + ")");
	}

	at(y*width + x) = c;
}

void MatrixBuffer::drawText(int x, int y, int offset, const Color& c,
	const std::string& str) {
	
	auto width = getWidth(), height = getHeight();

	const int CHAR_WIDTH = 5;
	const int CHAR_HEIGHT = 7;

	if(x < 0 || x >= width || y < 0 || y >= height) {
		std::cerr << "[Error] MatrixBuffer::drawText: Invalid coordinate" << std::endl;
		return;
	}

	int xPos = x, yPos = y;
	int strOffset = offset/(CHAR_WIDTH+1),
		chrOffset = offset % (CHAR_WIDTH+1);

	for(int i = strOffset; i < str.length(); ++i) {
		int fontIndex = CHAR_WIDTH*(str[i] - 32);

		for(int j = (i==strOffset)*chrOffset; j < CHAR_WIDTH && xPos < width; ++j) {
			unsigned char line = Font5x7[fontIndex + j];

			for(int pix = 0; pix < CHAR_HEIGHT; ++pix) {
				if(line & (1 << pix)) {
					set(xPos, yPos+pix, c);
				}
			}
			xPos++;
		}
		xPos++;
	}
}
