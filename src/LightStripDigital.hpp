#pragma once

#include "LightStrip.hpp"


class LightStripDigital;

class LightBufferDigital : public LightBuffer
{
public:
	LightBufferDigital(LightStripDigital *);

	LightBufferDigital operator<<(size_t shamt);
	LightBufferDigital operator>>(size_t shamt);
	
	size_t getSize() const;

	Color getColor(size_t) const;
	void setColor(size_t, const Color&);
};


class LightStripDigital : public LightStrip
{
public:
	LightStripDigital(size_t);

	std::unique_ptr<LightBuffer> getBuffer() override;
private:
	friend class LightBufferDigital;
};
