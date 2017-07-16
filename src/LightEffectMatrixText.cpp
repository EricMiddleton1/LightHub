#include "LightEffectMatrixText.hpp"
#include "LightStripMatrix.hpp"

LightEffectMatrixText::LightEffectMatrixText()
	:	LightEffect({LightStrip::Type::Matrix},
		{{"color", Color(255, 255, 255)}, {"text", std::string("Text")},
		{"speed", 10., Parameter::ValidatorGreater(0)},
		{"height", 0., Parameter::ValidatorRange(0, 255)}})
	,	pos{0} {
}

void LightEffectMatrixText::tick() {
	pos += getParameter("speed").getValue().getDouble() / 50.;

	//Handle overflow
	if(pos < 0.)
		pos = 0.;
}

void LightEffectMatrixText::updateStrip(std::shared_ptr<LightStrip> strip) {
	auto buffer = LightBuffer_cast<LightBufferMatrix>(strip->getBuffer());
	
	std::string text(getParameter("text").getValue().getString());
	Color c(getParameter("color").getValue().getColor());
	unsigned int height = std::min(getParameter("height").getValue().getDouble(),
		buffer->getHeight() - 7.);

	size_t curPos = (size_t)pos % (6*text.length() + buffer->getWidth());

	buffer->setAll({});
	
	int x = buffer->getWidth() - 1 - curPos;
	unsigned int offset = 0;
	if(x < 0) {
		offset = curPos - buffer->getWidth() + 1;
		x = 0;
	}

	buffer->drawText(x, height, offset, c, text);
}
