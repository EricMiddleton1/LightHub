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

void Rhopalia::threadRoutine() {
	ioService.run();
}

void Rhopalia::startUpdateTimer() {
	//Set the update timer
	updateTimer.expires_from_now(boost::posix_time::milliseconds(UPDATE_PERIOD));
	updateTimer.async_wait(std::bind(&Rhopalia::cbUpdateTimer, this, std::placeholders::_1));

/*	updateTimer.async_wait([this](const boost::system::error_code& ec) {
		if(ec) {
			//There was an error
			std::cout << "[Error] Rhopalia::cbUpdateTimer: error with deadline_timer: "
				<< ec.message() << std::endl;
		}
		else {
			std::cout <<  "[Info] Rhopalia::cbUpdateTimer: Timer finished" << std::endl;
			//Send the refresh signal
			sigRefreshTimer();
		}

		//Reset the timer
		startUpdateTimer();
	});*/
}

void Rhopalia::cbUpdateTimer(const boost::system::error_code& ec) {
	if(ec) {
		//There was an error
		std::cout << "[Error] Rhopalia::cbUpdateTimer: error with deadline_timer: "
			<< ec.message() << std::endl;
	}
	else {
		//Send the refresh signal
		sigRefreshTimer();
	}

	//Reset the timer
	startUpdateTimer();
}
