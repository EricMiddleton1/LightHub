#include "LightHub.hpp"

#include "LightStripAnalog.hpp"
#include "LightStripDigital.hpp"
#include "LightStripMatrix.hpp"

#include <chrono>

using namespace std;
using namespace boost::asio;

LightHub::LightHub(uint16_t _port, uint32_t _discoveryPeriod)
	:	socket(ioService, ip::udp::v4())
	,	port{_port}
	,	discoveryTimer(ioService, std::chrono::milliseconds(_discoveryPeriod),
		[this](){ discover(); })	{

	socket.set_option(socket_base::broadcast(true));
	socket.set_option(socket_base::reuse_address(true));

	ioWork.reset(make_unique<io_service::work>(ioService));

	asyncThread = std::thread(LightHub::threadRoutine, this);

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
	sendDatagram(ip::address_v4::broadcast(),
		Packet::NodeInfo().asDatagram());
}

void LightHub::sendDatagram(const ip::address& addr, const vector<uint8_t>& data) {
	lock_guard<mutex> sendLock(sendMutex);

	sendQueue.push_back(data);

	socket.async_send_to(buffer(sendQueue.back()), ip::udp::endpoint(addr, port),
		[this](boost::system::error_code& ec, size_t bytesTransferred) {
			lock_guard<mutex> sendLock(sendMutex);
			sendQueue.pop_front();

			if(ec) {
				cerr << "[Error] LightHub::cbSendDatagram: " << ec.message() << endl;
			}
		});
}

std::vector<std::shared_ptr<LightNode>>::iterator LightHub::begin() {
	return nodes.begin();
}

std::vector<std::shared_ptr<LightNode>>::iterator LightHub::end() {
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
		std::cout << "[Error] Failed to receive from UDP socket" << std::endl;
	}
	else {
		try {
			Packet p = Packet({readBuffer.begin(), readBuffer.begin() + bytesTransferred));

			switch(p.getID()) {
				case Packet::ID::NodeInfoResponse: {
					auto data = p.data();

					if(data.size() < 1) {
						cerr << "[Error] LightHub::handleReceive: Invalid payload size for NodeInfoResponse: "
							<< data.size() << endl;
					}
					else {
						string name{data.begin()+1, data.end()};

						for(int i = 0; i < data[0]; ++i) {
							sendDatagram(receiveEndpoint.address(), Packet::LightInfo(i).asDatagram);
						}

						if(nodeMap.find(receiveEndpoint.address()) != nodeMap.end()) {
							nodeMap.insert(receiveEndpoint.address();
						}

						cout << "[Info] LightHub::handleReceive: Node '" << name << "' discovered" << endl;
					}
				}
				break;

				case Packet::ID::LightInfoResponse: {
					auto node = nodeMap.find(receiveEndpoint.address());
					if(node == nodeMap.end()) {
						cerr << "[Info] LightHub::handleReceive: Received LightInfoResponse from node not "
							"in map" << endl;
					}
					else {
						if(data.size() < 2) {
							cerr << "[Error] LightHub::handleReceive: Invalid payload size for "
								"LightInfoResponse: " << data.size() << endl;
						}
						else {
							auto ledCount = Packet::parse16(data.begin());
							string name{data.begin()+1, data.end()};

							lights.emplace_back(endpoint.address(), p.getLightID(), name, ledCount);

							cout << "[Info] LightHub::handleReceive: 

					}
				}
				break;

			}
		}
		catch(const exception& e) {
			std::cout << "[Error] LightHub::handleReceive: " << e.what() << std::endl;
		}
		
		std::shared_ptr<LightNode> sendNode;

		//try to find the node associated with this packet
		try {
			sendNode = getNodeByAddress(receiveEndpoint.address());

			//Let the node handle the packet
			sendNode->receivePacket(p);
		}
		catch(const Exception& e) {
			if(e.getErrorCode() == LIGHT_HUB_NODE_NOT_FOUND) {
					//The sender is not in the list of connected nodes
					
				if(p.getID() == Packet::INFO) {
					auto payload = p.getPayload();
					if(payload.size() < 4) {
						std::cout << "[Error] LightHub::handleReceive: Info payload less than 4 "
							"bytes" << std::endl;
					}
					else {
						uint8_t analogCount = payload[0],
							digitalCount = payload[1],
							matrixCount = payload[2];
						
						if(payload.size() < (4 + 2*(digitalCount+matrixCount))) {
							throw Exception(LIGHT_HUB_INVALID_PAYLOAD,
								"LightHub::handleReceive: Expected payload at least "
								+ std::to_string(4 + 2*(digitalCount+matrixCount)) + " bytes, but "
								"is only " + std::to_string(payload.size()) + " bytes");
						}

						std::vector<std::shared_ptr<LightStrip>> strips;

						for(size_t i = 0; i < analogCount; ++i) {
							strips.emplace_back(std::make_shared<LightStripAnalog>());
						}

						for(size_t i = 0; i < digitalCount; ++i) {
							uint16_t size = (payload[3 + 2*i] << 8) | (payload[4 + 2*i]);

							strips.emplace_back(std::make_shared<LightStripDigital>(size));
						}

						for(size_t i = 0; i < matrixCount; ++i) {
							uint8_t width = payload[3 + 2*digitalCount + 2*i],
								height = payload[4 + 2*digitalCount + 2*i];

							std::cout << "Matrix: " << (int)width << ", " << (int)height << std::endl;

							strips.emplace_back(std::make_shared<LightStripMatrix>(width, height));
						}

						std::string name(payload.begin() + 3 + 2*(digitalCount+matrixCount),
							payload.end());
						
						auto newNode = std::make_shared<LightNode>(ioService, name, strips,
							receiveEndpoint);

						//Store the new node
						nodes.push_back(newNode);

						//Send a signal
						sigNodeDiscover(newNode);
					}
				}
			}
			else {
				//Some other error occurred
				std::cout << "[Error] LightHub::handleReceive: Exception thrown: "
					<< e.what() << std::endl;
			}
		}
	}

	startListening();
}

void LightHub::updateLights() {
	for(auto& node : nodes) {
		node->update();
	}
}
