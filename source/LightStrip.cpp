#include "LightStrip.hpp"

LightStrip::LightStrip() {
}

LightStrip::LightStrip(int size) {
	for(int i = 0; i < size; i++) {
		pixels.push_back(Color());
	}
}

Color LightStrip::getPixel(int i) {
	if(i < 0 || i >= (int)pixels.size()) {
		throw Exception(LIGHTSTRIP_INVALID_INDEX,
			"[LightStrip::getPixel]Error: invalid index");
	}

	return pixels[i];
}

void LightStrip::setPixel(int i, const Color& c) {
	if(i < 0 || i >= (int)pixels.size()) {
		throw Exception(LIGHTSTRIP_INVALID_INDEX,
			"[LightStrip::setPixel]Error: invalid index");
	}

	pixels[i] = c;
}

std::vector<Color>& LightStrip::getPixels() {
	return pixels;
}

int LightStrip::getSize() {
	return pixels.size();
}
