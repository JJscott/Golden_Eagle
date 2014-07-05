#include "ambition/Packet.hpp"

namespace ambition {
	class HelloPacket : public Packet {
		public:
			const static short ID = 1;
			static Packet constructPacket(Packet* p) { }
	};
};