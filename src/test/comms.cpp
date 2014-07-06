#include "gtest/gtest.h"
#include "ambition/Server.hpp"
#include "ambition/Client.hpp"

using namespace ambition;

// TODO: Write tests for Client v Server coms in here.
TEST(Comms, RawSocket) {
	ClientSocket cs;
	ListenSocket ls;
	
	cs.begin_connect("127.0.0.1", 8119, 1000);
	cs.on_connected.wait();
	EXPECT_TRUE(cs.connected());
}