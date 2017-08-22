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

	capture.pause();
}

std::unique_ptr<DisplayBuffer> Display::getBuffer() {
	auto buffer = std::make_unique<DisplayBuffer>(*this);

	{
		std::unique_lock<std::mutex> bufferLock(bufferMutex);

		buffers.push_back(buffer.get());

		if(buffers.size() == 1) {
			std::cout << "[Info] Display: Starting capture" << std::endl;

			capture.resume();
		}
	}

	return buffer;
}

void Display::releaseBuffer(const DisplayBuffer* buffer) {
	std::unique_lock<std::mutex> bufferLock(bufferMutex);

	auto found = std::find(buffers.begin(), buffers.end(), buffer);

	if(found == buffers.end()) {
		throw std::runtime_error("Display::releaseBuffer: buffer not found in vector");
	}

	buffers.erase(found);

	if(buffers.empty()) {
		std::cout << "[Info] Display: Stopping capture" << std::endl;

		capture.pause();
	}
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



	if(newTopOffset < topOffset) {
		std::cout << "[Info] New offsets: " << newTopOffset << ", "
			<< newBottomOffset <<  " (from " << topOffset << ", " << bottomOffset
			<< ")" << std::endl;

		topOffset = newTopOffset;
	}
	if(newBottomOffset < bottomOffset) {
		std::cout << "[Info] New offsets: " << newTopOffset << ", "
			<< newBottomOffset <<  " (from " << topOffset << ", " << bottomOffset
			<< ")" << std::endl;

		bottomOffset = newBottomOffset;
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
	else {
		offsetTime = 0;
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
		if( (data[index] > BLACK_LEVEL) || (data[index+1] > BLACK_LEVEL)
			|| (data[index+2] > BLACK_LEVEL) ) {
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

	if(count == 0) {
		std::cout << "[Error] Display::getAverageColor: Divide by 0 ("
			<< x1 << ", " << x2 << ", " << y1 << ", " << y2 << ")"
			<< std::endl;
		return {};
	}

	return {r/count, g/count, b/count};
}
