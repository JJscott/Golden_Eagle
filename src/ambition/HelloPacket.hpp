#include "ambition/Packet.hpp"

namespace ambition {
	namespace packets {
		class cs_hello_packet : public Packet {
		public:
			cs_hello_packet(int client_version) {}
			uint16_t get_id() { return packet_id::client_to_server_hello; }
			void handle() { };
		};

		class cs_hello_packet_factory : public packet_factory {
			Packet* deserialise(byte_buffer::reader& bb) { return new cs_hello_packet(bb.get<uint16_t>()); }
		};

		class sc_hello_packet : public Packet {
		public:
			sc_hello_packet(int server_version) {}
			uint16_t get_id() { return 02; }
			void handle() { };
			
		};

		class sc_hello_packet_factory : public packet_factory {
			Packet* deserialise(byte_buffer::reader& bb) { return new sc_hello_packet(bb.get<uint16_t>()); }
		};
	}
};