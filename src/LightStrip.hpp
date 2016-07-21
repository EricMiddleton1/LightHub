#pragma once

#include <vector>
#include <mutex>
#include <iostream> //For debugging

#include "Color.hpp"
#include "Exception.hpp"

class LightStrip
{
public:
	static const int LIGHTSTRIP_INVALID_INDEX = 0x0010;
	LightStrip();
	LightStrip(int size);
	
	Color getPixel(int i);
	void setPixel(int i, const Color& c);
	void setAll(const Color& c);

	std::vector<Color>& getPixels();

	int getSize();

private:
	std::vector<Color> pixels;
};
