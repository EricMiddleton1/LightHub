#include "ILightEffect.hpp"


ILightEffect::ILightEffect() {
}

ILightEffect::~ILightEffect() {
}

void ILightEffect::addNode(std::shared_ptr<LightNode> node) {
	//TODO: Make sure that the node isn't already in the vector

	nodes.push_back(node);
}

void ILightEffect::removeNode(std::shared_ptr<LightNode> node) {
	//Find the node in the vector
	auto found = std::find_if(std::begin(nodes), std::end(nodes),
		[&node](const std::shared_ptr<LightNode>& listNode) {
			return node.get() == listNode.get(); //Compare by address
		});

	if(found == std::end(nodes)) {
		//We didn't find the node
		throw Exception(EXCEPTION_LIGHT_EFFECT_NODE_NOT_FOUND,
			"ILightEffect::removeNode: node not found in vector");
	}
	
	//Remove the node
	nodes.erase(found);
}

std::vector<std::shared_ptr<LightNode>>::iterator
	ILightEffect::begin() {
	
	return std::begin(nodes);
}

std::vector<std::shared_ptr<LightNode>>::iterator
	ILightEffect::end() {
	
	return std::end(nodes);
}
