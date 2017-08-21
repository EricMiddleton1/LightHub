#include "Rhopalia.hpp"

#include <chrono>

Rhopalia::Rhopalia()
	:	hub(SEND_PORT, RECV_PORT)
	,	workUnit(std::make_unique<boost::asio::io_service::work>(ioService))
	,	updateTimer(ioService, std::chrono::milliseconds(UPDATE_PERIOD),
			[this]() { update(); }) {
}

Rhopalia::~Rhopalia() {
	//Stop the work associated with the ioService
	workUnit.reset();
}

void Rhopalia::run() {
	ioService.run();
}

void Rhopalia::addEffect(const std::shared_ptr<LightEffect>& effect) {
	effects.push_back(effect);
}

std::vector<std::shared_ptr<LightNode>>::iterator Rhopalia::nodeBegin() {
	return hub.begin();
}

std::vector<std::shared_ptr<LightNode>>::iterator Rhopalia::nodeEnd() {
	return hub.end();
}

void Rhopalia::update() {
	for(auto& effect : effects) {
		effect->update();
	}

	hub.updateLights();
}
