#include "Matrix.hpp"

#include <stdexcept>

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
		throw std::invalid_argument("MatrixBuffer::get: Invalid coordinate");
	}

	return at(y*width + x);
}

void MatrixBuffer::set(uint8_t x, uint8_t y, const Color& c) {
	auto width = getWidth(), height = getHeight();

	if( (x >= width) || (y >= height) ) {
		throw std::invalid_argument("MatrixBuffer::get: Invalid coordinate");
	}

	at(y*width + x) = c;
}
