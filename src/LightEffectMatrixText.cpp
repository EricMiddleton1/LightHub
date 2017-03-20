#include "LightEffectMatrixText.hpp"
#include "LightStripMatrix.hpp"

LightEffectMatrixText::LightEffectMatrixText()
	:	ILightEffect({LightStrip::Type::Matrix})
	,	pos{0} {
}

void LightEffectMatrixText::setText(const std::string& _text) {
	text = _text;
	pos = 0;
}

void LightEffectMatrixText::setColor(const Color& _c) {
	c = _c;
}

void LightEffectMatrixText::tick() {
	static size_t tick;

	tick = (tick + 1) % 2;

	if(!tick) {
		pos++;
	}
}

void LightEffectMatrixText::updateStrip(std::shared_ptr<LightStrip> strip) {
	auto buffer = LightBuffer_cast<LightBufferMatrix>(strip->getBuffer());
	size_t curPos = pos % (6*text.length() + buffer->getWidth());

	buffer->setAll({});
	
	int x = 0, y = 0, offset = 0;

	x = buffer->getWidth() - 1 - curPos;
	if(x < 0) {
		offset = curPos - buffer->getWidth() + 1;
		x = 0;
	}

	buffer->drawText(x, y, offset, c, text);
}
