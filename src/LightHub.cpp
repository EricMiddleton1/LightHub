#include "LightHub.hpp"

#include "Packet.hpp"
#include "Matrix.hpp"

using namespace std;
using namespace boost::asio;

const int LightHub::LIGHT_TIMEOUT = 4;

std::string toString(const std::vector<uint8_t>& data) {
	std::string str("{");

	if(!data.empty()) {
		for(int i = 0; i < (data.size()-1); ++i) {
			str += to_string(static_cast<int>(data[i])) + ", ";
		}
		str += to_string(static_cast<int>(data[data.size()-1]));
	}
	str += "}";

	return str;
}

LightNode::LightNode(const string& _name)
	:	name{_name} {
}

LightHub::LightHub(uint16_t _port, uint32_t _discoveryPeriod)
	:	ioWork{make_unique<io_service::work>(ioService)}
	,	socket(ioService, ip::udp::v4())
	,	port{_port}
	,	discoveryTimer(ioService, std::chrono::milliseconds(_discoveryPeriod),
		[this](){ handleDiscoveryTimer(); })	{

	socket.set_option(socket_base::broadcast(true));
	socket.set_option(socket_base::reuse_address(true));

	asyncThread = std::thread([this]() { threadRoutine(); });

	std::cout << "[Info] LightHub::LightHub: Now listening for packets"
		<< std::endl;
	
	//Post constructor setup (on local thread)
	ioService.post([this]() {
		discover();

		startListening();
	});
}

LightHub::~LightHub() {
	//Delete the work unit, allow io_service.run() to return
	ioWork.reset();

	asyncThread.join();
}

void LightHub::threadRoutine() {
	ioService.run();
}


void LightHub::discover() {
	auto data = Packet::NodeInfo().asDatagram();

	sendDatagram(ip::address_v4::broadcast(),
		data);
}

void LightHub::sendDatagram(const ip::address& addr, const vector<uint8_t>& data) {
	lock_guard<mutex> sendLock(sendMutex);

	sendQueue.push_back(data);

	socket.async_send_to(buffer(sendQueue.back()), ip::udp::endpoint(addr, port),
		[this](const boost::system::error_code& ec, size_t bytesTransferred) {
			lock_guard<mutex> sendLock(sendMutex);
			sendQueue.pop_front();

			if(ec) {
				cerr << "[Error] LightHub::cbSendDatagram: " << ec.message() << endl;
			}
		});
}

LightHub::NodeIterator LightHub::begin() const {
	return nodes.begin();
}

LightHub::NodeIterator LightHub::end() const {
	return nodes.end();
}

size_t LightHub::getNodeCount() const {
	return nodes.size();
}

void LightHub::startListening() {
	//Start the async receive
	socket.async_receive_from(buffer(readBuffer),
		receiveEndpoint,
		[this](const boost::system::error_code& ec, size_t bytesTransferred) {
			handleReceive(ec, bytesTransferred);
		});
}

void LightHub::handleReceive(const boost::system::error_code& ec,
	size_t bytesTransferred) {
	
	if(ec) {
		cerr << "[Error] Failed to receive from UDP socket" << endl;
	}
	else {
		try {
			Packet p{vector<uint8_t>{readBuffer.begin(), readBuffer.begin() +
				bytesTransferred}};
			auto data = p.data();

			switch(p.getID()) {
				case Packet::ID::NodeInfoResponse: {
					if(data.size() < 1) {
						cerr << "[Error] LightHub::handleReceive: Invalid payload size for "
							"NodeInfoResponse: " << data.size() << endl;
					}
					else {
						string name{data.begin()+1, data.end()};

						for(int i = 0; i < data[0]; ++i) {
							sendDatagram(receiveEndpoint.address(), Packet::LightInfo(i).asDatagram());
						}

						if(nodes.find(receiveEndpoint.address()) == nodes.end()) {
							nodes.emplace(receiveEndpoint.address(), name);
						}
					}
				}
				break;

				case Packet::ID::LightInfoResponse: {
					auto node = nodes.find(receiveEndpoint.address());
					if(node == nodes.end()) {
						cerr << "[Info] LightHub::handleReceive: Received LightInfoResponse from "
						"node not in map" << endl;
					}
					else {
						if(p.data().size() < 4) {
							cerr << "[Error] LightHub::handleReceive: Invalid payload size for "
								"LightInfoResponse: " << p.data().size() << endl;
						}
						else {
							auto ledCount = Packet::parse16(p.data().begin());
							int type = data[0];
							string name{data.begin()+3, data.end()};

							auto light = find_if(node->second.lights.begin(), node->second.lights.end(),
								[&name](const std::shared_ptr<Light>& light) {
									return light->getName() == name;
								});

							if(light == node->second.lights.end()) {
								if(type == 0) {
									node->second.lights.emplace_back(make_shared<Light>(*this,
										node->second, receiveEndpoint.address(), p.getLightID(), name,
										Packet::parse16(data.begin()+1)));
								}
								else {
									node->second.lights.emplace_back(make_shared<Matrix>(*this,
										node->second, receiveEndpoint.address(), p.getLightID(), name,
										data[1], data[2]));
								}
								node->second.timeouts.push_back(LIGHT_TIMEOUT);

								sigLightDiscover(node->second.lights.back());
							}
							else {
								auto matrix = dynamic_pointer_cast<Matrix>(*light);
								int existingType = !!matrix;

								if( (existingType != type) ||
									(type == 0 && (*light)->getSize() != Packet::parse16(data.begin()+1)) ||
									(type == 1 && ( matrix->getWidth() != data[1] ||
										matrix->getHeight() != data[2] ) ) ) {
								
									node->second.timeouts.erase(node->second.timeouts.begin() +
										(light - node->second.lights.begin()));
									node->second.lights.erase(light);

									cout << "[Info] LightHub::handleReceive: Previously connected light "
										<< node->second.name << "/" << name << " has changed LED count"
										<< endl;
								}
								else {
									node->second.timeouts[light - node->second.lights.begin()] =
										LIGHT_TIMEOUT;
								}
							}
						}
					}
				}
				break;

				default:
					cerr << "[Error] Unexpected message ID received: " << static_cast<int>(p.getID())
						<< endl;
				break;
			}
		}
		catch(const exception& e) {
			cerr << "[Error] LightHub::handleReceive: " << e.what() << endl;
		}
	}
	
	startListening();
}

void LightHub::update(Light& light) {
	sendDatagram(light.getAddress(),
		Packet::UpdateColor(light.getLightID(),
			light.getHuePeriod(), light.getSatPeriod(), light.getValPeriod(),
			light.getPixels()).asDatagram());
}

void LightHub::handleDiscoveryTimer() {
	for(auto& nodeEntry : nodes) {
		auto& node = nodeEntry.second;

		for(int i = 0; i < node.timeouts.size(); ++i) {
			if(--node.timeouts[i] == 0) {
				cout << "[Info] LightHub: Light "
					<< node.name << "/" << node.lights[i]->getName() << " has timed out"
					<< endl;

				node.lights.erase(node.lights.begin() + i);
				node.timeouts.erase(node.timeouts.begin() + i);
				--i;
			}
		}
	}

	discover();
}
