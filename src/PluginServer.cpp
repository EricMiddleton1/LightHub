#include "PluginServer.hpp"

using namespace std;
using boost::asio;

PluginServer::PluginServer(uint16_t _port)
	:	listenSocket(ioService, ip::tcp::endpoint(
