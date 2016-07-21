#include "LightEffectSolid.hpp"


LightEffectSolid::LightEffectSolid() {

}

LightEffectSolid::LightEffectSolid(const Color& initialColor)
	:	color(initialColor) {
}

void LightEffectSolid::addNode(const std::shared_ptr<LightNode>& node) {
	nodes.push_back(node);

	//Add listener for state changes
	node->addListener(LightNode::STATE_CHANGE,
		std::bind(&LightEffectSolid::slotStateChange, this, std::placeholders::_1,
		std::placeholders::_3));

	//set the node's color
	node->getLightStrip().setAll(color);

	std::cout << "[Info] LightEffectSolid::addNode: Setting new node '"
		<< node->getName() << "' to color " << color.toString() << std::endl;

	//Release the strip
	node->releaseLightStrip();
}

void LightEffectSolid::setColor(const Color& c) {
	if(c != color) {
		std::cout << "[Info] LightEffectSolid::setColor: Changing color from "
			<< color.toString() << " to " << c.toString() << std::endl;

		color = c;

		//Iterate through the nodes and update the color
		for(auto& node : nodes) {
			node->getLightStrip().setAll(color);

			//Release the strip
			node->releaseLightStrip();
		}
	}
}

Color LightEffectSolid::getColor() const {
	return color;
}

void LightEffectSolid::update() {
	//There's nothing to update here
}

void LightEffectSolid::slotStateChange(LightNode* node,
	LightNode::State_e newState) {

	if(newState == LightNode::CONNECTED) {
		//Update the pixels
		node->getLightStrip().setAll(color);

		//Release the strip
		node->releaseLightStrip();
	}
}
