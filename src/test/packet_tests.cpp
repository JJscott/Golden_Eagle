#include "gtest/gtest.h"
#include "ambition/ByteBuffer.hpp"
using namespace ambition;

#include <ctime>

const int test_size = 30;

// char* get_random_bytes(int n) {
// 	srand(time(NULL));
// 	char* bytes = new char[n];
// 	for(int i = 0; i < n; i++) {
// 		bytes[i] = rand() % 256;
		
// 	}
// 	return bytes;
// }

// TEST(byte_buffer, Constructor) {
// 	byte_buffer p;
// 	EXPECT_EQ(p.size(), 0);
// }

// TEST(byte_buffer, SingleByte) {
// 	byte_buffer p;
// 	char random_byte = get_random_bytes(1)[0];
// 	p.add(random_byte);
// 	EXPECT_EQ(p.size(), 1);
// 	EXPECT_EQ(p.get_byte(), random_byte);
// 	EXPECT_EQ(p.size(), 0);
// }

// TEST(byte_buffer, SingleBytePeek) {
// 	byte_buffer p;
// 	char random_byte = get_random_bytes(1)[0];
// 	p.add(random_byte);
// 	EXPECT_EQ(p.size(), 1);
// 	EXPECT_EQ(p.peek_byte(), random_byte);
// 	EXPECT_EQ(p.size(), 1);
// }

// TEST(byte_buffer, MultiByte) {
// 	byte_buffer p;
// 	char* testData = get_random_bytes(test_size);
// 	p.add(testData, test_size);
// 	EXPECT_EQ(p.size(), test_size);
// 	char* gathered_bytes = p.get_bytes(test_size);
// 	EXPECT_EQ(memcmp(gathered_bytes, testData, test_size), 0);
// 	EXPECT_EQ(p.size(), 0);
// }

// TEST(byte_buffer, MultiBytePeek) {
// 	byte_buffer p;
// 	char* testData = get_random_bytes(test_size);
// 	p.add(testData, test_size);
// 	EXPECT_EQ(p.size(), test_size);
// 	EXPECT_EQ(p.peek_byte(), testData[0]);
// 	EXPECT_EQ(p.size(), test_size);
// 	EXPECT_EQ(memcmp(p.peek_bytes(test_size), testData, test_size), 0);
// 	EXPECT_EQ(p.size(), test_size);
// }

