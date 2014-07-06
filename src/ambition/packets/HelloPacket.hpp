#include "ambition/Packet.hpp"

namespace ambition {
	namespace packets {
		class HelloPacket : public Packet {
			public:
				HelloPacket() : Packet(1) {}
				virtual inline void execute_handler(Session* st, ByteBuffer* dp) { 
				}
		};
	}
};