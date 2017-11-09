#include "Drawing.hpp"

#include "Light.hpp"
#include "Color.hpp"

#include "font5x7.h"

void Drawing::drawText(LightBuffer& buffer, int width, int height, int x, int y, int offset, const Color& c,
	const std::string& str) {

	const int CHAR_WIDTH = 5;
	const int CHAR_HEIGHT = 7;

	if(x < 0 || x >= width || y < 0 || y >= height) {
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
					buffer[(yPos+pix)*width + xPos] = c;
				}
			}
			xPos++;
		}
		xPos++;
	}
}
