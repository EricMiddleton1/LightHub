#include "Display.hpp"

#include <stdexcept>
#include <chrono>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <string>

using namespace std::literals::chrono_literals;

Display::Display()
	:	ConfigurableObject{ { {"frame rate", 30} } } {
	std::mutex wait;
	std::condition_variable cv;
	bool ready = false;

	std::unique_lock<std::mutex> lock(wait);

	capture = SL::Screen_Capture::CreateScreeCapture([this, &ready, &cv]() {
		auto monitors = SL::Screen_Capture::GetMonitors();

		rawImage.width = monitors.begin()->Width;
		rawImage.height = monitors.begin()->Height;

		ready = true;
		cv.notify_one();

		return SL::Screen_Capture::GetMonitors();
	}).onNewFrame([this](const SL::Screen_Capture::Image& img,
		const SL::Screen_Capture::Monitor& monitor) {

		std::unique_lock<std::mutex> lock(mutex);
		this->rawImage.set(img);
	}).start_capturing();

	cv.wait(lock, [&ready]{return ready;});
	
	capture.setFrameChangeInterval(1s /
		(double)getParameter("frame rate").getValue().getInt());

}

Display::RawImage::RawImage()
	:	data{nullptr} 
	,	topOffset{0}
	,	bottomOffset{0}
	,	offsetTime{0} {
}

Display::RawImage::~RawImage() {
	if(data != nullptr) {
		delete[] data;
	}
}

void Display::RawImage::set(const SL::Screen_Capture::Image& img) {
	width = img.Bounds.right - img.Bounds.left;
	height = img.Bounds.bottom - img.Bounds.top;

	if(data == nullptr) {
		data = new unsigned char[height*SL::Screen_Capture::RowStride(img)];
	}

	SL::Screen_Capture::Extract(img, reinterpret_cast<char*>(data),
		height*SL::Screen_Capture::RowStride(img));

	//Check top and bottom offsets
	auto newTopOffset = calcTopOffset();
	auto newBottomOffset = calcBottomOffset();



	if(newTopOffset <= topOffset) {
		topOffset = newTopOffset;
		offsetTime = 0;
	}
	if(newBottomOffset <= bottomOffset) {
		bottomOffset = newBottomOffset;
		offsetTime = 0;
	}

	if( (newTopOffset > topOffset) && (newBottomOffset > bottomOffset) ) {

		offsetTime++;

		if(offsetTime >= MAX_OFFSET_TIME) {
		std::cout << "[Info] New offsets: " << newTopOffset << ", "
			<< newBottomOffset <<  " (from " << topOffset << ", " << bottomOffset
			<< ")" << std::endl;

			offsetTime = 0;
			topOffset = newTopOffset;
			bottomOffset = newBottomOffset;
		}
	}
}

Display::Coordinate Display::RawImage::calcTopOffset() const {
	Coordinate y;
	for(y = 0; y < MAX_OFFSET; ++y) {
		if(!blackRow(y)) {
			break;
		}
	}

	return y;
}

Display::Coordinate Display::RawImage::calcBottomOffset() const {
	Coordinate y;
	for(y = 0; y < MAX_OFFSET; ++y) {
		if(!blackRow(height-y-1)) {
			break;
		}
	}

	return y;
}

bool Display::RawImage::blackRow(Coordinate y) const {
	for(Coordinate x = 0; x < width; ++x) {
		auto index = getIndex(x, y);
		if( (data[index] > 32) || (data[index+1] > 32) || (data[index+2] > 32) ) {
			return false;
		}
	}

	return true;
}

Display::Coordinate Display::getWidth() const {
	return rawImage.width;
}

Display::Coordinate Display::getHeight() const {
	return rawImage.height;
}

Display::Coordinate Display::getTopOffset() const {
	return rawImage.topOffset;
}

Display::Coordinate Display::getBottomOffset() const {
	return rawImage.bottomOffset;
}

Color Display::get(Coordinate x, Coordinate y) const {
	if( (x < 0) || (x >= rawImage.width)
		|| (y < 0) || (y >= rawImage.height) ) {
		throw std::runtime_error("Display::get: Coordinates out of bounds ("
			+ std::to_string(x) + ", " + std::to_string(y) + ")");
	}
	
	int index = y*rawImage.width*4 + x*4;

	std::unique_lock<std::mutex> lock(mutex);
	
	if(rawImage.data == nullptr) {
		return {};
	}

	return {rawImage.data[index+2], rawImage.data[index+1], rawImage.data[index]};
}

int Display::RawImage::getIndex(Coordinate x, Coordinate y) const {
	return y*width*4 + x*4;
}

Color Display::getAverageColor(Coordinate x1, Coordinate y1, Coordinate x2,
	Coordinate y2) const {
	
	std::unique_lock<std::mutex> lock(mutex);

	if(rawImage.data == nullptr) {
		return {};
	}

	unsigned long long r{0}, g{0}, b{0};

	for(int j = y1; j < y2; ++j) {
		for(int i = x1; i < x2; ++i) {
			auto index = rawImage.getIndex(i, j);

			r += rawImage.data[index+2];
			g += rawImage.data[index+1];
			b += rawImage.data[index];
		}
	}

	unsigned long long count = (x2-x1+1) * (y2-y1+1);

	return {r/count, g/count, b/count};
}
