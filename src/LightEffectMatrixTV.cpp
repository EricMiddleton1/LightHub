#include "LightEffectMatrixTV.hpp"

#include "Matrix.hpp"

LightEffectMatrixTV::LightEffectMatrixTV(const std::shared_ptr<Display>& _display)
	:	LightEffect{ {} }
	,	display{_display} {
}

void LightEffectMatrixTV::tick() {
}

bool LightEffectMatrixTV::validateLight(const std::shared_ptr<Light>& light) {
	return true;
}

void LightEffectMatrixTV::updateLight(std::shared_ptr<Light>& light) {
	auto buffer = buffer_cast<MatrixBuffer>(light->getBuffer());

	auto mWidth = buffer->getWidth();
	auto mHeight = buffer->getHeight();
	
	int dispWidth = display->getWidth(),
		dispHeight = display->getHeight();
	
	for(int j = 0; j < mHeight; ++j) {
		for(int i = 0; i < mWidth; ++i) {
			auto x = (j % 2) ? mWidth - i - 1 : i;
			buffer->set(x, j, display->getAverageColor(i*dispWidth/mWidth,
				j*dispHeight/mHeight, (i+1)*dispWidth/mWidth, (j+1)*dispHeight/mHeight));
		}
	}
}
