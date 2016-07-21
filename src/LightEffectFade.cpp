#include "LightEffectFade.hpp"


LightEffectFade::LightEffectFade(float brightness, float speed)
	:	brightness{brightness}
	,	speed{speed} {

}

void LightEffectFade::addNode(const std::shared_ptr<LightNode>& node) {
	nodes.push_back(node);

	//Add listener for state changes
	node->addListener(LightNode::STATE_CHANGE,
		std::bind(&LightEffectFade::slotStateChange, this, std::placeholders::_1,
		std::placeholders::_3));

	//set the node's color
	node->getLightStrip().setAll(color);

	std::cout << "[Info] LightEffectSolid::addNode: Setting new node '"
		<< node->getName() << "' to color " << color.toString() << std::endl;

	//Release the strip
	node->releaseLightStrip();
}

void LightEffectFade::update() {
	hue = std::fmod(hue + speed, 360.f);
	color = Color::HSV(hue, 1.f, brightness);

	//Update all of the nodes
	for(auto& node : nodes) {
		node->getLightStrip().setAll(color);
		
		//Release the light strip resource
		node->releaseLightStrip();
	}
}

void LightEffectFade::slotStateChange(LightNode* node,
	LightNode::State_e newState) {

	if(newState == LightNode::CONNECTED) {
		//Update the pixels
		node->getLightStrip().setAll(color);

		//Release the strip
		node->releaseLightStrip();
	}
}
