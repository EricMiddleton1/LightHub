#pragma once

#include <string>

#include "ILightEffect.hpp"
#include "Color.hpp"

class LightEffectMatrixText : public ILightEffect
{
public:
	LightEffectMatrixText();

	void setText(const std::string& text);
	void setColor(const Color& c);

private:
	virtual void tick() override;
	virtual void updateStrip(std::shared_ptr<LightStrip>) override;

	Color c;
	std::string text;
	size_t pos;
};
