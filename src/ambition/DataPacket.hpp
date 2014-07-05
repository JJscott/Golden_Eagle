#ifndef DATAPACKET_HPP
#define DATAPACKET_HPP

#include <deque>

namespace ambition {
	class DataPacket {
		std::deque<char> bytestream;
	public:
		int size() { return bytestream.size(); }
		void add(char b) {
			bytestream.push_back(b);
		}

		char peek_byte() {
			return bytestream.front();
		}

		char get_byte() {
			char t = bytestream.front();
			bytestream.pop_front();
			return t;
		}

		void add(char* bts, size_t n) {
			for(size_t i = 0; i < n; i++) {
				bytestream.push_back(bts[i]);
			}
		}

		char* peek_bytes(size_t n) {
			if(n > bytestream.size()) return NULL;
			char* ret = new char[n];
			int i = 0;
			for(auto it : bytestream) {
				ret[i++] = it;
			}
			return ret;
		}

		char* get_bytes(size_t n) {
			if(n > bytestream.size()) return NULL;
			char* ret = new char[n];
			for(size_t i = 0; i < n; i++) {
				ret[i] = bytestream.front();
				bytestream.pop_front();
			}
			return ret;
		}
	};
}

#endif 