#pragma once

#include "LightStrip.hpp"


class LightStripAnalog;

class LightBufferAnalog : public LightBuffer
{
public:
	LightBufferAnalog(LightStripAnalog *);

	Color getColor() const;
	void setColor(const Color&);
};


class LightStripAnalog : public LightStrip
{
public:
	LightStripAnalog();

	std::unique_ptr<LightBuffer> getBuffer() override;
private:
	friend class LightBufferAnalog;
};
