#include "gtest/gtest.h"
#include "ambition/Packet.hpp"

#include <ctime>

const int test_size = 30;

char* get_random_bytes(int n) {
	srand(time(NULL));
	char* bytes = new char[n];
	for(int i = 0; i < n; i++) {
		bytes[i] = rand() % 256;
		
	}
	return bytes;
}

TEST(Packet, Constructor) {
	Packet p;
	EXPECT_EQ(p.size(), 0);
}

TEST(Packet, SingleByte) {
	Packet p;
	char random_byte = get_random_bytes(1)[0];
	p.add(random_byte);
	EXPECT_EQ(p.size(), 1);
	EXPECT_EQ(p.get_byte(), random_byte);
	EXPECT_EQ(p.size(), 0);
}

TEST(Packet, SingleBytePeek) {
	Packet p;
	char random_byte = get_random_bytes(1)[0];
	p.add(random_byte);
	EXPECT_EQ(p.size(), 1);
	EXPECT_EQ(p.peek_byte(), random_byte);
	EXPECT_EQ(p.size(), 1);
}

TEST(Packet, MultiByte) {
	Packet p;
	char* testData = get_random_bytes(test_size);
	p.add(testData, test_size);
	EXPECT_EQ(p.size(), test_size);
	char* gathered_bytes = p.get_bytes(test_size);
	EXPECT_EQ(memcmp(gathered_bytes, testData, test_size), 0);
	EXPECT_EQ(p.size(), 0);
}

TEST(Packet, MultiBytePeek) {
	Packet p;
	char* testData = get_random_bytes(test_size);
	p.add(testData, test_size);
	EXPECT_EQ(p.size(), test_size);
	EXPECT_EQ(p.peek_byte(), testData[0]);
	EXPECT_EQ(p.size(), test_size);
	EXPECT_EQ(memcmp(p.peek_bytes(test_size), testData, test_size), 0);
	EXPECT_EQ(p.size(), test_size);
}
