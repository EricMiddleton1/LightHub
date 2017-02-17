#include "PluginServer.hpp"

using namespace std;
using boost::asio;

PluginServer::PluginServer(uint16_t _port)
	:	ioWork(make_unique<io_service::work>(ioService))
	,	listenSocket(ioService, ip::tcp::endpoint(ip::tcp::v4(), _port))
	,	asyncThread(std::bind(PluginServer::threadRoutine, this)) {

}
