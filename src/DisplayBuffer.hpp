#pragma once

#include "Color.hpp"

class Display;

class DisplayBuffer {
public:
	using Coordinate = int;

	DisplayBuffer(Display&);
	DisplayBuffer(const DisplayBuffer&) = delete;
	DisplayBuffer(DisplayBuffer&&) = delete;

	~DisplayBuffer();

	Coordinate getWidth() const;
	Coordinate getHeight() const;

	Coordinate getTopOffset() const;
	Coordinate getBottomOffset() const;

	Color get(Coordinate x, Coordinate y) const;
	Color getAverageColor(Coordinate x1, Coordinate y1,
		Coordinate x2, Coordinate y2) const;

private:
	friend class Display;

	Display& display;
};
