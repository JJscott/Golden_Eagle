#ifndef PACKET_HPP
#define PACKET_HPP

#include <ambition/ByteBuffer.hpp>
#include <ambition/Server.hpp>

namespace ambition {
	 
	struct PacketID {
	        enum {
	                c2s_init,

	                last // packet id not found
	        };
	};
	 
	template <unsigned ID>
	class PacketImpl {	};

	class Packet;

	
	class PacketVisitor {
	public:
        virtual void visit(const Packet &) = 0;
       
        virtual void visit(const PacketImpl<PacketID::c2s_init> &p);
       
        virtual ~PacketVisitor() { }
	};
		 
	class Packet {
	        template <unsigned I>
	        struct deserialize_impl {
	                static Packet * go(unsigned id, byte_buffer::reader &r) {
	                        if (I == id) {
	                                return PacketImpl<I>::deserialize(r);
	                        } else {
	                                return deserialize_impl<I + 1>::go(id, r);
	                        }
	                }
	        };
	       
	        template <>
	        struct deserialize_impl<PacketID::last> {
	                static Packet * go(unsigned id, byte_buffer::reader &r) {
	                        // throw exception - no packet of this id
	                }
	        };
	       
	public:
        static Packet * deserialize(const byte_buffer &bb) {
                auto r = bb.read();
                unsigned id = r.get<uint16_t>();
                return deserialize_impl<0>::go(id, r);
        }

        virtual void accept(PacketVisitor &v) const =0;
        virtual ~Packet() { }
	};
	 
	 
	template <>
	class PacketImpl<PacketID::c2s_init> : public Packet {
		uint16_t client_version = 0;
	public:
        static Packet * deserialize(byte_buffer::reader &r) {
        	return new Packet<PacketID::c2s_init>(r.get<uint16_t>());
        }
       
        void accept(PacketVisitor &v) const override {
                v.visit(*this);
        }
	};

	class PacketHandler : public PacketVisitor {

	};
}

#endif