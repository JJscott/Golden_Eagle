#ifndef BYTEBUFFER_HPP
#define BYTEBUFFER_HPP

#ifndef _WIN32
#include <netinet/in.h>
#endif
#include <deque>

namespace ambition {
	class ByteBuffer {
		std::deque<char> bytestream;
	public:
		ByteBuffer() {}
		ByteBuffer(const char* s, int sz) {
			add_bytes(s, sz);
		}

		int size() const { return bytestream.size(); }

		const char* data() const {
			char* ret = new char[size()];
			int i = 0;
			for(auto it : bytestream) {
				ret[i++] = it;
			}
			return ret;
		}

		void add_byte(char b) {
			bytestream.push_back(b);
		}

		void add(char b) {
			add_byte(b);
		}

		char peek_byte() {
			return bytestream.front();
		}

		char get_byte() {
			char t = bytestream.front();
			bytestream.pop_front();
			return t;
		}

		void add_bytes(const char* bts, size_t n) {
			for(size_t i = 0; i < n; i++) {
				bytestream.push_back(bts[i]);
			}
		}

		void add(const char* bts, size_t n) {
			add_bytes(bts, n);
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

		void add_short(uint16_t s) {
			add_byte(s & 0x00FF);
			add_byte((s & 0xFF00) >> 8);
		}

		uint16_t get_short() {
    		uint16_t result = 0;
    		char tmp = get_byte();
    		result = (result<<8) + get_byte();
    		result = (result<<8) + tmp;
    		return result;
		}

		void add_string(std::string msg) {
			std::cout << "adding string, msg.length(): " << msg.length() << std::endl;
			add_short(msg.length());
			for(size_t i = 0; i < msg.length(); i++) {
				bytestream.push_back(msg[i]);
			}
		}

		const char* get_string() {
			size_t len = get_short();
			return get_bytes(len);

		}
	};
}

#endif 