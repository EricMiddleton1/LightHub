#pragma once

#include <string>
#include <cstdint>

class Color {
public:
	Color();
	Color(uint8_t r, uint8_t g, uint8_t b);

	Color operator=(const Color& c);
	bool operator!=(const Color& rhs) const;
	bool operator==(const Color& rhs) const;
	Color operator*(double rhs) const;

	std::string toString() const;

	uint8_t getRed() const;
	uint8_t getGreen() const;
	uint8_t getBlue() const;

	void setRed(uint8_t red);
	void setGreen(uint8_t green);
	void setBlue(uint8_t blue);

	uint8_t getHue() const;
	uint8_t getSat() const;
	uint8_t getVal() const;

	Color filter(const Color&, double);

	void gammaCorrect(double gamma);

	static Color HSV(uint8_t hue, uint8_t sat, uint8_t val);

private:
	float getHueF() const;
	float getSatF() const;
	float getValF() const;

	uint8_t r, g, b;
};
