#include "LightEffectMatrixClock.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>

#include "Matrix.hpp"

LightEffectMatrixClock::LightEffectMatrixClock()
	:	LightEffect{{{"color", Color(0, 0, 255)}}} {
}

bool LightEffectMatrixClock::validateLight(const std::shared_ptr<Light>& light) {
	return true;
}

void LightEffectMatrixClock::tick() {
	std::time_t curTime;

	auto chronoTime = std::chrono::system_clock::now();
	curTime = std::chrono::system_clock::to_time_t(chronoTime);
	std::tm tm = *std::localtime(&curTime);

	unsigned int hour = (tm.tm_hour % 12);
	std::string minStr = std::to_string(tm.tm_min);
	if(tm.tm_min < 10)
		minStr = "0" + minStr;
	timeStr = std::to_string((hour == 0) ? 12 : hour) + ":" + minStr;
}

void LightEffectMatrixClock::updateLight(std::shared_ptr<Light>& light) {
	Color c = getParameter("color").getValue().getColor();

	auto buffer = buffer_cast<MatrixBuffer>(light->getBuffer());

	buffer->setAll({});

	unsigned int strWidth = 6 * timeStr.length() - 1,
		strHeight = 7;

	buffer->drawText((buffer->getWidth() - strWidth)/2,
		(buffer->getHeight() - strHeight)/2, 0, c, timeStr);
}
