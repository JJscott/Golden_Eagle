#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>

#include "ambition/Log.hpp"
#include "ambition/Packet.hpp"
#include "ambition/packets/HelloPacket.hpp"

using namespace ambition;

namespace ambition {
	using packet_id_type = ushort;
	using packet_id_map = std::map<packet_id_type, Packet (*)(Packet*)>;

	class MessageFactory { 
		packet_id_map pm;

	public:
		MessageFactory() {
			pm[HelloPacket::ID] = HelloPacket::constructPacket;
		}
	};

	class Server {
		bool isPublic = false;
	public:
		Server(bool);
		void start();
	};
}
#endif