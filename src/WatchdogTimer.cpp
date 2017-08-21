#include "WatchdogTimer.hpp"

#include <stdexcept>

WatchdogTimer::WatchdogTimer(boost::asio::io_service& _ioService,
	const std::chrono::microseconds& _timeout, const TimeoutHandler& _handler)
	:	ioService{_ioService}
	,	timeout{_timeout}
	,	handler{_handler}
	,	timer{ioService}
	,	state{State::Stopped} {
		if(!handler) {
			throw std::invalid_argument("WatchdogTimer: Invalid handler");
		}
}

void WatchdogTimer::start() {
	if(state == State::Running) {
		return;
	}

	setTimer();

	state = State::Running;
}

void WatchdogTimer::stop() {
	if(state == State::Stopped) {
		return;
	}

	timer.cancel();

	state = State::Stopped;
}

void WatchdogTimer::feed() {
	if(state == State::Stopped) {
		start();
	}
	else {
		timer.cancel();
		setTimer();
	}
}

void WatchdogTimer::setTimer() {
	timer.expires_from_now(timeout);
	timer.async_wait([this](const boost::system::error_code& error) {
		if(!error) {
			state = State::Stopped;

			handler();
		}
	});
}
