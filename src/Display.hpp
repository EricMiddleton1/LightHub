#pragma once

#include <mutex>

#include "ScreenCapture.h"
#include "ConfigurableObject.hpp"
#include "Color.hpp"

class Display : public ConfigurableObject
{
public:
	using Coordinate = int;

	Display();
	
	Coordinate getWidth() const;
	Coordinate getHeight() const;

	Color get(Coordinate x, Coordinate y) const;
	Color getAverageColor(Coordinate x1, Coordinate y1,
		Coordinate x2, Coordinate y2) const;

private:
	int getIndex(Coordinate x, Coordinate y) const;

	SL::Screen_Capture::ScreenCaptureManager capture;
	
	struct RawImage {
		RawImage();
		~RawImage();

		void set(const SL::Screen_Capture::Image& img);

		Coordinate width, height;
		unsigned char *data;
	} rawImage;

	mutable std::mutex mutex;
};
