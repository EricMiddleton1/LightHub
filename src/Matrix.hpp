#pragma once

#include "Light.hpp"

#include <memory>

class MatrixBuffer;

class Matrix : public Light {
public:
	Matrix(LightHub&, LightNode&, const boost::asio::ip::address&, uint8_t lightID,
		const std::string& name, uint8_t width, uint8_t height);
	~Matrix();

	uint8_t getWidth() const;
	uint8_t getHeight() const;

	std::unique_ptr<LightBuffer> getBuffer() override;

	static bool isMatrix(const std::shared_ptr<Light>& light);
private:
	friend class MatrixBuffer;

	uint16_t width, height;
};

class MatrixBuffer : public LightBuffer {
public:
	MatrixBuffer(Matrix&);

	uint8_t getWidth() const;
	uint8_t getHeight() const;

	Color get(uint8_t x, uint8_t y) const;
	void set(uint8_t x, uint8_t y, const Color& c);

	void drawText(int x, int y, int offset, const Color& c, const std::string& str);
};

template<typename Derived, typename Base, typename Del>
std::unique_ptr<Derived>
buffer_cast(std::unique_ptr<Base, Del>&& ptr) {
	if(Derived *result = dynamic_cast<Derived*>(ptr.get())) {
		ptr.release();
		return std::unique_ptr<Derived>(result);
	}
	else {
		return std::unique_ptr<Derived>(nullptr);
	}
}
