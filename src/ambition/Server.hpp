#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>

#include "ambition/Ambition.hpp"
#include "ambition/ListenSocket.hpp"
#include "ambition/Log.hpp"
#include "ambition/Packet.hpp"
#include "ambition/packets/HelloPacket.hpp"

namespace ambition {
	class MessageFactory {
		std::map<uint16_t, ambition::packets::Packet*> pid_map;
	public:
		void add_handler(ambition::packets::Packet* p) {
			// TODO: Check if p->get_id() in pid_map.keys
			pid_map[p->get_id()] = p;
		}
	};

	class Server {
		bool isPublic = false;
		ListenSocket* lsocket;
		MessageFactory mfactory;
	public:
		Server();
		Server(bool);
		uint16_t listen_port();
		void start();		
		int get_version();
	};
}
#endif