#pragma once

#include <vector>
#include <memory>

#include "Exception.hpp"
#include "LightNode.hpp"


//Forward declaration
class Rhopalia;

class ILightEffect
{
public:
	static const uint16_t EXCEPTION_LIGHT_EFFECT_NODE_NOT_FOUND = 0x0020;
	static const uint16_t EXCEPTION_LIGHT_EFFECT_UNSUPPORTED_TYPE = 0x0021;
	static const uint16_t EXCEPTION_LIGHT_EFFECT_NODE_ALREADY_CONNECTED = 0x0022;

	ILightEffect(const std::vector<LightNode::Type>&);
	virtual ~ILightEffect();

	virtual void addNode(const std::shared_ptr<LightNode>&);
	virtual void removeNode(std::shared_ptr<LightNode>);


	virtual std::vector<std::shared_ptr<LightNode>>::iterator begin();
	virtual std::vector<std::shared_ptr<LightNode>>::iterator end();

protected:
	friend Rhopalia;

	//Slots
	virtual void update() = 0;

	std::vector<std::shared_ptr<LightNode>> nodes;
	std::vector<LightNode::Type> supportedTypes;
};
