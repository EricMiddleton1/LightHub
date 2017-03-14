#pragma once

#include <vector>
#include <array>
#include <mutex>
#include <memory>
#include <iostream> //For debugging

#include "Color.hpp"
#include "Exception.hpp"


class LightBuffer;

class LightStrip
{
public:
	enum class Type {
		Analog = 1,
		Digital = 2,
		Matrix = 3
	};

	static const int INVALID_INDEX = 0x0010;
	LightStrip(Type type, size_t size);

	size_t getID() const;

	Type getType() const;
	
	std::vector<Color> getPixels() const;

	size_t getSize() const;

	virtual std::unique_ptr<LightBuffer> getBuffer() = 0;

protected:
	friend class LightBuffer;

	size_t id;

	Type type;

	std::vector<Color> pixels, pixelBuffer;
	mutable std::mutex pixelMutex, bufferMutex;

private:
	static size_t globalCount;
	std::mutex countMutex;
};

class LightBuffer
{
public:
	LightBuffer(LightStrip*);
	virtual ~LightBuffer();

	LightStrip::Type getType() const;

	void setAll(const Color& c);

protected:
	LightStrip *strip;
	std::vector<Color>& getPixelBuffer();
	const std::vector<Color>& getPixelBufferConst() const;
};

template<typename Derived, typename Base, typename Del>
std::unique_ptr<Derived, Del>
LightBuffer_cast(std::unique_ptr<Base, Del>&& ptr) {
	if(Derived *result = dynamic_cast<Derived*>(ptr.get())) {
		ptr.release();
		return std::unique_ptr<Derived, Del>(result, std::move(ptr.get_deleter()));
	}
	else {
		return std::unique_ptr<Derived, Del>(nullptr, ptr.get_deleter());
	}
}
