#include "LightEffectMatrixText.hpp"
#include "Matrix.hpp"

LightEffectMatrixText::LightEffectMatrixText()
	:	LightEffect({{"color", Color(255, 255, 255)}, {"text", std::string("Text")},
		{"speed", 10., Parameter::ValidatorGreater(0)},
		{"height", 0., Parameter::ValidatorRange(0, 255)}}, true)
	,	pos{0} {
}

bool LightEffectMatrixText::validateLight(const std::shared_ptr<Light>& light) {
	return true;
}

void LightEffectMatrixText::tick() {
	pos += getParameter("speed").getValue().getDouble() / 50.;

	//Handle overflow
	if(pos < 0.)
		pos = 0.;
}

void LightEffectMatrixText::updateLight(std::shared_ptr<Light>& light) {
	auto buffer = buffer_cast<MatrixBuffer>(light->getBuffer());
	
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
/*
	std::cout << "[Info] Drawing text '" << text << " at (" << x << ", "
		<< height << ", " << offset <<") with color " << c.toString() << std::endl;
*/
	buffer->drawText(x, height, offset, c, text);
}
