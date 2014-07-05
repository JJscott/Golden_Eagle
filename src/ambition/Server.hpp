#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>

#include "ambition/Ambition.hpp"
#include "ambition/Socket.hpp"
#include "ambition/Log.hpp"
#include "ambition/Packet.hpp"
#include "ambition/packets/HelloPacket.hpp"

namespace ambition {
	using packet_id_type = uint16_t;
	using packet_id_map = std::map<packet_id_type,  Packet* (*)(Packet*)>

	class MessageFactory { 
		packet_id_map pm;

	public:
		MessageFactory() {
			pm[HelloPacket::ID] = HelloPacket::dewirePacket;
		}
	};

	class Server {
		bool isPublic = false;
		ListenSocket* lsocket;
	public:
		Server();
		Server(bool);
		uint16_t listen_port();
		void start();		
		int get_version();
	};
}
#endif