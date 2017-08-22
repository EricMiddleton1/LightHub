#include "LightEffectDigitalTV.hpp"

#include "LightStripDigital.hpp"

LightEffectDigitalTV::LightEffectDigitalTV(const std::shared_ptr<Display>& _display)
	:	LightEffect{ {LightStrip::Type::Digital},
		{{"width", 68}, {"height", 36}, {"depth", 64}, {"filter", 0.3}} }
	,	display{_display} 
	,	edge{2*68 + 2*36} {
	
	onStart = [this]() { displayBuffer = display->getBuffer(); };
	onStop = [this]() { displayBuffer.reset(); };
}

void LightEffectDigitalTV::tick() {
	int stripWidth = getParameter("width").getValue().getInt();
	int stripHeight = getParameter("height").getValue().getInt();
	int depth = getParameter("depth").getValue().getInt();
	auto filter = getParameter("filter").getValue().getDouble();

	auto y0 = displayBuffer->getTopOffset();
	auto width = displayBuffer->getWidth();
	auto height = displayBuffer->getHeight() - displayBuffer->getBottomOffset();
	auto adjHeight = height - y0;

	for(int x = 0; x < stripWidth; ++x) {
		edge[x].filter(displayBuffer->getAverageColor(x*width/stripWidth, y0,
			(x+1)*width/stripWidth, y0 + depth), filter);
		edge[2*stripWidth+stripHeight-x-1].filter(
			displayBuffer->getAverageColor(x*width/stripWidth, height-depth,
			(x+1)*width/stripWidth, height), filter);
	}
	for(int y = 0; y < stripHeight; ++y) {
		edge[stripWidth+y].filter(displayBuffer->getAverageColor(
			width-depth, y0 + y*adjHeight/stripHeight, width,
			y0 + (y+1)*adjHeight/stripHeight), filter);
		edge[2*stripWidth+2*stripHeight-y-1].filter(
			displayBuffer->getAverageColor(0, y0 + y*adjHeight/stripHeight, depth,
				y0 + (y+1)*adjHeight/stripHeight),
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
