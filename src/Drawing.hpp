#pragma once

#include <string>

class LightBuffer;
class Color;

namespace Drawing {
	void drawText(LightBuffer&, int width, int height, int x, int y, int offset,
		const Color& c, const std::string& str);

}
