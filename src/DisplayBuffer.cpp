#include "DisplayBuffer.hpp"

#include "Display.hpp"

DisplayBuffer::DisplayBuffer(Display& _display)
	:	display{_display} {
}

DisplayBuffer::~DisplayBuffer() {
	display.releaseBuffer(this);
}

DisplayBuffer::Coordinate DisplayBuffer::getWidth() const {
	return display.getWidth();
}

DisplayBuffer::Coordinate DisplayBuffer::getHeight() const {
	return display.getHeight();
}

DisplayBuffer::Coordinate DisplayBuffer::getTopOffset() const {
	return display.getTopOffset();
}

DisplayBuffer::Coordinate DisplayBuffer::getBottomOffset() const {
	return display.getBottomOffset();
}

Color DisplayBuffer::get(DisplayBuffer::Coordinate x, DisplayBuffer::Coordinate y) const {
	return display.get(x, y);
}

Color DisplayBuffer::getAverageColor(Coordinate x1, Coordinate y1,
	Coordinate x2, Coordinate y2) const {
	
	return display.getAverageColor(x1, y1, x2, y2);
}
