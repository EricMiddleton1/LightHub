#pragma once

#include <cmath>
#include <algorithm>
#include <string>

typedef unsigned char byte;

class Color {
public:
	Color();
	Color(int r, int g, int b);

	Color operator=(const Color& c);
	bool operator!=(const Color& rhs) const;
	bool operator==(const Color& rhs) const;
	Color operator*(double rhs) const;

	std::string toString() const;

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

	Color filter(const Color&, double);

	void gammaCorrect(double gamma);

	void print() const;

	static Color HSL(float hue, float saturation, float lightness);
	static Color HSV(float hue, float saturation, float value);

private:
	int r, g, b;
};
