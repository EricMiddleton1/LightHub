#pragma once

#include <chrono>
#include <functional>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

class PeriodicTimer {
public:
	using TimerHandler = std::function<void(void)>;

	PeriodicTimer(boost::asio::io_service& service,
		const std::chrono::microseconds& period, const TimerHandler& handler);

private:
	void cbTimer(const boost::system::error_code& error);
	void resetTimer();

	boost::asio::io_service& ioService;
	std::chrono::microseconds period;
	TimerHandler handler;

	boost::asio::steady_timer timer;
};
