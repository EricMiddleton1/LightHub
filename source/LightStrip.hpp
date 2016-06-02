#ifndef LIGHTSTRIP_HPP
#define LIGHTSTRIP_HPP

#include <vector>

#include "Color.hpp"
#include "Exception.hpp"
#include "ErrorCode.hpp"

class LightStrip
{
public:
	LightStrip();
	LightStrip(int size);
	
	Color getPixel(int i);
	void setPixel(int i, const Color& c);

	std::vector<Color>& getPixels();

	int getSize();

private:
	std::vector<Color> pixels;
};



#endif //LIGHTSTRIP_HPP
