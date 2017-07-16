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

		std::cout << "Display(): " << rawImage.width << ", " << rawImage.height
			<< std::endl;

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
	:	data{nullptr} {
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
}

Display::Coordinate Display::getWidth() const {
	return rawImage.width;
}

Display::Coordinate Display::getHeight() const {
	return rawImage.height;
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

int Display::getIndex(Coordinate x, Coordinate y) const {
	return y*rawImage.width*4 + x*4;
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
			auto index = getIndex(i, j);

			r += rawImage.data[index+2];
			g += rawImage.data[index+1];
			b += rawImage.data[index];
		}
	}

	unsigned long long count = (x2-x1+1) * (y2-y1+1);

	return {r/count, g/count, b/count};
}
