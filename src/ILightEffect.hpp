#pragma once

#include <vector>
#include <memory>

#include "Exception.hpp"
#include "LightNode.hpp"

class ILightEffect
{
public:
	static const uint16_t EXCEPTION_LIGHT_EFFECT_NODE_NOT_FOUND = 0x0020;

	virtual void addNode(std::shared_ptr<LightNode>);
	virtual void removeNode(std::shared_ptr<LightNode>);


	virtual std::vector<std::shared_ptr<LightNode>>::iterator begin();
	virtual std::vector<std::shared_ptr<LightNode>>::iterator end();

	virtual void forceUpdate() = 0;

private:
	std::vector<std::shared_ptr<LightNode>> nodes;

};
