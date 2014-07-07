#ifndef PACKET_HPP
#define PACKET_HPP

#include <ambition/ByteBuffer.hpp>
#include <ambition/Error.hpp>

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
        virtual void visit(const PacketImpl<PacketID::c2s_init> &p) = 0;
       
        virtual ~PacketVisitor() { }
	};
		 
	class Packet {
        template <unsigned I, typename Dummy = void>
        struct deserialize_impl {
                static Packet * go(unsigned id, byte_buffer::reader &r) {
                        if (I == id) {
                                return PacketImpl<I>::deserialize(r);
                        } else {
                                return deserialize_impl<I + 1>::go(id, r);
                        }
                }
        };
       
        template <typename Dummy>
        struct deserialize_impl<PacketID::last, Dummy> {
                static Packet * go(unsigned id, byte_buffer::reader &r) {
                    throw new network_error(error::neterr_packet_id_not_found, "Unrecognised packet ID");
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
		uint16_t client_version_impl = 0;
	public:
        static Packet * deserialize(byte_buffer::reader &r) {
        	return new PacketImpl<PacketID::c2s_init>(r.get<uint16_t>());
        }

        PacketImpl<PacketID::c2s_init>(uint16_t nc) : client_version_impl(nc) {}
       
        void accept(PacketVisitor &v) const override {
                v.visit(*this);
        }

        uint16_t client_version() { return client_version_impl; }
	};

	class PacketHandler : public PacketVisitor {
		void visit(const Packet& p) { /* what goes here? */ }
		void visit(const PacketImpl<PacketID::c2s_init> &p) {}
	};
}

void ambition::PacketVisitor::visit(const PacketImpl<PacketID::c2s_init> &p) {
    visit(static_cast<const Packet &>(p));
}

#endif