#pragma once

#include <cmath>
#include <algorithm>
#include <string>

typedef unsigned char byte;

class Color {
public:
	Color();
	Color(int r, int g, int b);

	Color operator=(Color c);
	bool operator!=(Color rhs);
	bool operator==(Color rhs);

	std::string toString();

	byte getRed() const;
	byte getGreen() const;
	byte getBlue() const;

	void setRed(byte red);
	void setGreen(byte green);
	void setBlue(byte blue);

	float getHue() const;
	float getHSLSaturation() const;
	float getLightness() const;

	float getHSVSaturation() const;
	float getValue() const;

	void print() const;

	static Color HSL(float hue, float saturation, float lightness);
	static Color HSV(float hue, float saturation, float value);

private:
	int r, g, b;
};
