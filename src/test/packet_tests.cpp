#include "gtest/gtest.h"
#include "ambition/DataPacket.hpp"
using namespace ambition;

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

TEST(DataPacket, Constructor) {
	DataPacket p;
	EXPECT_EQ(p.size(), 0);
}

TEST(DataPacket, SingleByte) {
	DataPacket p;
	char random_byte = get_random_bytes(1)[0];
	p.add(random_byte);
	EXPECT_EQ(p.size(), 1);
	EXPECT_EQ(p.get_byte(), random_byte);
	EXPECT_EQ(p.size(), 0);
}

TEST(DataPacket, SingleBytePeek) {
	DataPacket p;
	char random_byte = get_random_bytes(1)[0];
	p.add(random_byte);
	EXPECT_EQ(p.size(), 1);
	EXPECT_EQ(p.peek_byte(), random_byte);
	EXPECT_EQ(p.size(), 1);
}

TEST(DataPacket, MultiByte) {
	DataPacket p;
	char* testData = get_random_bytes(test_size);
	p.add(testData, test_size);
	EXPECT_EQ(p.size(), test_size);
	char* gathered_bytes = p.get_bytes(test_size);
	EXPECT_EQ(memcmp(gathered_bytes, testData, test_size), 0);
	EXPECT_EQ(p.size(), 0);
}

TEST(DataPacket, MultiBytePeek) {
	DataPacket p;
	char* testData = get_random_bytes(test_size);
	p.add(testData, test_size);
	EXPECT_EQ(p.size(), test_size);
	EXPECT_EQ(p.peek_byte(), testData[0]);
	EXPECT_EQ(p.size(), test_size);
	EXPECT_EQ(memcmp(p.peek_bytes(test_size), testData, test_size), 0);
	EXPECT_EQ(p.size(), test_size);
}

