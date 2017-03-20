#pragma once

#include <string>

#include "LightStrip.hpp"


class LightStripMatrix;

class LightBufferMatrix : public LightBuffer
{
public:
	LightBufferMatrix(LightStripMatrix *);
	
	size_t getWidth() const;
	size_t getHeight() const;

	Color getColor(size_t x, size_t y) const;
	void setColor(size_t x, size_t y, const Color&);

	void drawText(size_t x, size_t y, size_t offset, const Color& c, const std::string& str);
};


class LightStripMatrix : public LightStrip
{
public:
	LightStripMatrix(size_t, size_t);

	size_t getWidth() const;
	size_t getHeight() const;

	std::unique_ptr<LightBuffer> getBuffer() override;
private:
	friend class LightBufferMatrix;
	size_t width, height;
};
