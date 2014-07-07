#ifndef PACKET_HPP
#define PACKET_HPP

#include <ambition/ByteBuffer.hpp>
#include <ambition/Server.hpp>

namespace ambition {
	namespace packets {
		enum packet_id {
			client_to_server_hello = 0,
			server_to_client_hello
		};

		class Packet {
		public:
			virtual inline uint16_t get_id()=0;
			virtual inline void handle()=0; // todo: needs arguments
		};

		class packet_factory {
			virtual inline Packet* deserialise(byte_buffer::reader&)=0;
		};
	}
}

#endif