#include "Rhopalia.hpp"


Rhopalia::Rhopalia()
	:	hub(SEND_PORT, RECV_PORT, DISCOVER_METHOD) {
	//TODO: Read these values from a config file
}

Rhopalia::~Rhopalia() {
}
