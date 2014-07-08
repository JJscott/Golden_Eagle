#include "RemoteGameServer.hpp"
#include "Log.hpp"
#include "Packet.hpp"

namespace ambition {
	RemoteGameServer::RemoteGameServer(std::string hostname, uint16_t port) {
		csocket.on_connected.attach([this](const SocketResult &sr) { return this->connection_complete(sr); });
		csocket.begin_connect(hostname, port, 5000);
	}

	bool RemoteGameServer::connection_complete(SocketResult sr) {
		if(csocket.connected()) {
			// ambition::PacketImpl<PacketID::c2s_init> p();
			// csocket.begin_send(p.serialize());

			// init_complete.notify(ActionResult());	
		}
		return false;
	}
}