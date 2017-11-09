#include "LightEffectMatrixTV.hpp"


LightEffectMatrixTV::LightEffectMatrixTV(const std::shared_ptr<Display>& _display)
	:	LightEffect{ {{"width", 32}, {"height", 24}} }
	,	display{_display} {
}

void LightEffectMatrixTV::tick() {
}

bool LightEffectMatrixTV::validateLight(const std::shared_ptr<Light>& light) {
	return light->getSize() == (getParameter("width").getValue().getInt() *
		getParameter("height").getValue().getInt());
}

void LightEffectMatrixTV::updateLight(std::shared_ptr<Light>& light) {
	auto buffer = light->getBuffer();

	auto mWidth = getParameter("width").getValue().getInt();
	auto mHeight = getParameter("height").getValue().getInt();
	
	int dispWidth = display->getWidth(),
		dispHeight = display->getHeight();
	
	for(int j = 0; j < mHeight; ++j) {
		for(int i = 0; i < mWidth; ++i) {
			buffer[mWidth*j + i] = 
				display->getAverageColor(i*dispWidth/mWidth, j*dispHeight/mHeight,
					(i+1)*dispWidth/mWidth, (j+1)*dispHeight/mHeight);
		}
	}
}
