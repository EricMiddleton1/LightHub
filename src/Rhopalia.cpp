#include "Rhopalia.hpp"


Rhopalia::Rhopalia()
	:	hub(SEND_PORT, RECV_PORT, DISCOVER_METHOD)
	,	workUnit(std::make_unique<boost::asio::io_service::work>(ioService))
	, asyncThread(std::bind(&Rhopalia::threadRoutine, this))
	,	updateTimer(ioService) {
	//TODO: Read these values from a config file
	
	startUpdateTimer();
}

Rhopalia::~Rhopalia() {
	//Stop the work associated with the ioService
	workUnit.reset();
	
	//Wait for the thread to finish
	asyncThread.join();
}

void Rhopalia::addEffect(const std::shared_ptr<ILightEffect>& effect) {
	effects.push_back(effect);
}

std::vector<std::shared_ptr<LightNode>>::iterator Rhopalia::nodeBegin() {
	return hub.begin();
}

std::vector<std::shared_ptr<LightNode>>::iterator Rhopalia::nodeEnd() {
	return hub.end();
}


void Rhopalia::threadRoutine() {
	ioService.run();
}

void Rhopalia::startUpdateTimer() {
	//Set the update timer
	updateTimer.expires_from_now(
		boost::posix_time::milliseconds(UPDATE_PERIOD));

	updateTimer.async_wait(std::bind(&Rhopalia::cbUpdateTimer, this,
		std::placeholders::_1));
}

void Rhopalia::cbUpdateTimer(const boost::system::error_code& ec) {
	//Restart the timer
	startUpdateTimer();

	if(ec) {
		//There was an error
		std::cout << "[Error] Rhopalia::cbUpdateTimer: error with "
			"deadline_timer: " << ec.message() << std::endl;
	}
	else {
		//Update the effects first
		for(auto& effect : effects) {
			effect->update();
		}

		//Then update the lights
		hub.updateLights();
	}
}
