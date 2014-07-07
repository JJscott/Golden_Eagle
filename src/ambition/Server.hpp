#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>

#include "ambition/Ambition.hpp"
#include "ambition/ListenSocket.hpp"
#include "ambition/Log.hpp"
#include "ambition/Packet.hpp"

namespace ambition {
	class Server {
		bool isPublic = false;
		ListenSocket* lsocket;
		PacketHandler handler;
	public:
		Server();
		Server(bool);
		uint16_t listen_port();
		void start();		
		int get_version();
	};
}
#endif