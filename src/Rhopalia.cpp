#include "Rhopalia.hpp"

#include <chrono>

Rhopalia::Rhopalia()
	:	hub(PORT)
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

LightHub::NodeIterator Rhopalia::nodeBegin() {
	return hub.begin();
}

LightHub::NodeIterator Rhopalia::nodeEnd() {
	return hub.end();
}

void Rhopalia::update() {
	for(auto& effect : effects) {
		effect->update();
	}
}
