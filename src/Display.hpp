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

	Coordinate getTopOffset() const;
	Coordinate getBottomOffset() const;

	Color get(Coordinate x, Coordinate y) const;
	Color getAverageColor(Coordinate x1, Coordinate y1,
		Coordinate x2, Coordinate y2) const;

private:


	SL::Screen_Capture::ScreenCaptureManager capture;
	
	struct RawImage {
		const int MAX_OFFSET = 200;
		const int MAX_OFFSET_TIME = 30;

		RawImage();
		~RawImage();
		
		int getIndex(Coordinate x, Coordinate y) const;

		void set(const SL::Screen_Capture::Image& img);

		Coordinate width, height;
		Coordinate topOffset, bottomOffset;

		int offsetTime;

		unsigned char *data;

	private:
		Coordinate calcTopOffset() const;
		Coordinate calcBottomOffset() const;
		bool blackRow(Coordinate y) const;
	} rawImage;

	mutable std::mutex mutex;
};
