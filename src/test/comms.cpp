#include "gtest/gtest.h"
#include "ambition/Server.hpp"
#include "ambition/Client.hpp"

// TODO: Write tests for Client v Server coms in here.
TEST(Comms, Hello) {
	ambition::Client c;
	ambition::Server s;
	c.target("localhost", s.listen_port());

	for(int i = 0; i < 3 && !c.connected(); i++)
		c.block_until_connected(1000);

	// EXPECT_TRUE(c.connected());
	// EXPECT_EQ(c.get_server_version(), s.get_version());

}