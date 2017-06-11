#include "LightEffectMatrixClock.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>
#include "LightStripMatrix.hpp"

LightEffectMatrixClock::LightEffectMatrixClock()
	:	LightEffect({LightStrip::Type::Matrix},
		{{"color", Color(0, 0, 255)}}) {
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

void LightEffectMatrixClock::updateStrip(std::shared_ptr<LightStrip> strip) {
	Color c = getParameter("color").getValue().getColor();

	auto buffer = LightBuffer_cast<LightBufferMatrix>(strip->getBuffer());

	buffer->setAll({});

	unsigned int width = buffer->getWidth(),
		height = buffer->getHeight();
	
	unsigned int strWidth = 6 * timeStr.length() - 1,
		strHeight = 7;

	buffer->drawText((width - strWidth)/2, (height - strHeight)/2, 0, c, timeStr);
}
