#include "LightEffectDigitalTV.hpp"

#include "LightStripDigital.hpp"

LightEffectDigitalTV::LightEffectDigitalTV(const std::shared_ptr<Display>& _display)
	:	LightEffect{ {LightStrip::Type::Digital},
		{{"width", 68}, {"height", 36}, {"depth", 64}, {"filter", 0.3}} }
	,	display{_display} 
	,	width{display->getWidth()}
	,	height{display->getHeight()}
	,	edge{2*68 + 2*36} {
}

void LightEffectDigitalTV::tick() {
	int stripWidth = getParameter("width").getValue().getInt();
	int stripHeight = getParameter("height").getValue().getInt();
	int depth = getParameter("depth").getValue().getInt();
	auto filter = getParameter("filter").getValue().getDouble();

	for(int x = 0; x < stripWidth; ++x) {
		edge[x].filter(display->getAverageColor(x*width/stripWidth, 0,
			(x+1)*width/stripWidth, depth), filter);
		edge[2*stripWidth+stripHeight-x-1].filter(
			display->getAverageColor(x*width/stripWidth, height-depth,
			(x+1)*width/stripWidth, height), filter);
	}
	for(int y = 0; y < stripHeight; ++y) {
		edge[stripWidth+y].filter(display->getAverageColor(
			width-depth, y*height/stripHeight, width, (y+1)*height/stripHeight), filter);
		edge[2*stripWidth+2*stripHeight-y-1].filter(
			display->getAverageColor(0, y*height/stripHeight, depth, (y+1)*height/stripHeight),
			filter);
	}
}

void LightEffectDigitalTV::updateStrip(std::shared_ptr<LightStrip> strip) {
	auto buffer = LightBuffer_cast<LightBufferDigital>(strip->getBuffer());

	int stripWidth = getParameter("width").getValue().getInt();
	int stripHeight = getParameter("height").getValue().getInt();

	if(buffer->getSize() != (2*stripWidth + 2*stripHeight)) {
		std::cout << "[Error] LightBufferDigitalTV::update: Invalid strip size ("
			<< "expected " << (2*stripWidth+2*stripHeight) << ", was actually "
			<< buffer->getSize() << ")" << std::endl;

		return;
	}
	
	for(int i = 0; i < buffer->getSize(); ++i) {
		buffer->setColor(i, edge[i]);
	}
}
