#include "Color.hpp"

#include <cmath>

Color::Color() {
	r = g = b = 0;
}

Color::Color(int r, int g, int b) {
	this->r = r;
	this->g = g;
	this->b = b;
}

Color Color::operator=(const Color& c) {
	r = c.r;
	g = c.g;
	b = c.b;

	return *this;
}

bool Color::operator!=(const Color& rhs) const {
	bool res = (r != rhs.r) || (g != rhs.g) || (b != rhs.b);

	return res;
}

bool Color::operator==(const Color& rhs) const {
	bool res = (r == rhs.r) && (g == rhs.g) && (b == rhs.b);

	return res;
}

Color Color::operator*(double rhs) const {
	return {r*rhs, g*rhs, b*rhs};
}

std::string Color::toString() const {
	return "(" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ")";
}

byte Color::getRed() const {
	return r;
}

byte Color::getGreen() const {
	return g;
}

byte Color::getBlue() const {
	return b;
}

Color Color::filter(const Color& other, double factor) {
	double invFactor = 1. - factor;

	r = r*factor + other.r*invFactor;
	g = g*factor + other.g*invFactor;
	b = b*factor + other.b*invFactor;

	return *this;
}

void Color::gammaCorrect(double gamma) {
	r = 255. * std::pow(r/255., gamma) + 0.5;
	g = 255. * std::pow(g/255., gamma) + 0.5;
	b = 255. * std::pow(b/255., gamma) + 0.5;
}

float Color::getHue() const {
	int min = std::min(r, std::min(g, b)), max = std::max(r, std::max(g, b));
	float chroma = max - min;
	float hprime, hue;

	if (chroma == 0)
		hprime = 0;
	else if (max == r)
		hprime = std::fmod(((float)g - b) / chroma, 6.);
	else if (max == g)
		hprime = ((float)b - r) / chroma + 2;
	else
		hprime = ((float)r - g) / chroma + 4;

	hue = 60.*hprime;

	if (hue < 0)
		hue += 360;
	else if (hue > 360)
		hue -= 360;

	return hue;
}

float Color::getHSLSaturation() const {
	float min = std::min(r, std::min(g, b)), max = std::max(r, std::max(g, b));
	float chroma = max - min;
	float saturation, lightness = getLightness();
	
	if (chroma == 0)
		saturation = 0;
	else
		saturation = (float)chroma / (1 - std::abs(2.*lightness - 1));

	return saturation / 255.;
}

float Color::getLightness() const {
	byte min = std::min(r, std::min(g, b)), max = std::max(r, std::max(g, b));

	return ((float)min + max) / (2. * 255.);
}

float Color::getHSVSaturation() const {
	float min = std::min(r, std::min(g, b)), max = std::max(r, std::max(g, b));
	float chroma = max - min;
	float saturation, value = getValue();

	if (chroma == 0)
		saturation = 0;
	else
		saturation = chroma / value;

	return saturation / 255.;
}

float Color::getValue() const {
	return std::max({ r, g, b }) / 255.f;
}

void Color::print() const {
	printf("(%d, %d, %d)\n", r, g, b);
}

Color Color::HSL(float hue, float saturation, float lightness) {
	float chroma, hprime, x, m, r, g, b;

	hue = std::fmod(hue, 360.);

	hprime = hue / 60.f;

	chroma = (1 - std::abs(2 * lightness - 1)) * saturation;
	x = (1 - std::abs(std::fmod(hprime, 2) - 1)) * chroma;

	if (hprime < 1) {
		r = chroma;
		g = x;
		b = 0;
	}
	else if (hprime < 2) {
		r = x;
		g = chroma;
		b = 0;
	}
	else if (hprime < 3) {
		r = 0;
		g = chroma;
		b = x;
	}
	else if (hprime < 4) {
		r = 0;
		g = x;
		b = chroma;
	}
	else if (hprime < 5) {
		r = x;
		g = 0;
		b = chroma;
	}
	else {
		r = chroma;
		g = 0;
		b = x;
	}

	m = lightness - chroma / 2.;

	r += m;
	g += m;
	b += m;

	return Color(255 * r, 255 * g, 255 * b);
}

Color Color::HSV(float hue, float saturation, float value) {
	float chroma, hprime, x, m, r, g, b;

	hue = std::fmod(hue, 360.);

	hprime = hue / 60.f;

	chroma = value * saturation;
	x = (1 - std::abs(std::fmod(hprime, 2) - 1)) * chroma;

	if (hprime < 1) {
		r = chroma;
		g = x;
		b = 0;
	}
	else if (hprime < 2) {
		r = x;
		g = chroma;
		b = 0;
	}
	else if (hprime < 3) {
		r = 0;
		g = chroma;
		b = x;
	}
	else if (hprime < 4) {
		r = 0;
		g = x;
		b = chroma;
	}
	else if (hprime < 5) {
		r = x;
		g = 0;
		b = chroma;
	}
	else {
		r = chroma;
		g = 0;
		b = x;
	}

	m = value - chroma;

	r += m;
	g += m;
	b += m;

	return Color(255 * r, 255 * g, 255 * b);
}
