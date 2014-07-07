#include "Packet.hpp"

virtual void visit(const PacketImpl<PacketID::c2s_init> &p) {
    visit(static_cast<const Packet &>(p));
}