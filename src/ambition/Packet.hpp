#ifndef PACKET_HPP
#define PACKET_HPP

#include "ambition/DataPacket.hpp"
#include "ambition/Server.hpp"

namespace ambition {
	class Session {
	public:
	};

	namespace packets {
		class Packet {
			uint16_t p_id;
		public:
			Packet(uint16_t n_p_id) : p_id(n_p_id) {}
			virtual inline uint16_t get_id() { return p_id; }
			virtual inline void execute_handler(Session* st, DataPacket* dp) =0;
		};
	}
}

#endif