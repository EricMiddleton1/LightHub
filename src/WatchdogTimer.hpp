#pragma once

#include <chrono>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

class WatchdogTimer {
public:
	using TimeoutHandler = std::function<void(void)>;

	enum class State {
		Stopped = 0,
		Running
	};

	WatchdogTimer(boost::asio::io_service& ioService,
		const std::chrono::microseconds& timeout, const TimeoutHandler& handler);

	void start();
	void stop();

	void feed();

	State getState() const;

private:
	void setTimer();

	boost::asio::io_service& ioService;
	std::chrono::microseconds timeout;
	TimeoutHandler handler;

	boost::asio::steady_timer timer;
	State state;
};
