#include "LightEffectMatrixTV.hpp"

#include "LightStripMatrix.hpp"

LightEffectMatrixTV::LightEffectMatrixTV(const std::shared_ptr<Display>& _display)
	:	LightEffect{ {LightStrip::Type::Matrix}, {} }
	,	display{_display} {
}

void LightEffectMatrixTV::tick() {
}

void LightEffectMatrixTV::updateStrip(std::shared_ptr<LightStrip> strip) {
	auto buffer = LightBuffer_cast<LightBufferMatrix>(strip->getBuffer());

	int dispWidth = display->getWidth(),
		dispHeight = display->getHeight();
	int mWidth = buffer->getWidth(),
		mHeight = buffer->getHeight();
	
	for(int j = 0; j < mHeight; ++j) {
		for(int i = 0; i < mWidth; ++i) {
			buffer->setColor(i, j,
				display->getAverageColor(i*dispWidth/mWidth, j*dispHeight/mHeight,
					(i+1)*dispWidth/mWidth, (j+1)*dispHeight/mHeight));
		}
	}
}
