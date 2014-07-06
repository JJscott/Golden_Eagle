#ifndef BYTEBUFFER_HPP
#define BYTEBUFFER_HPP

#ifndef _WIN32
#include <netinet/in.h>
#endif
#include <deque>

#include <cstdint>
#include <climits>
#include <cstring>
#include <cassert>
#include <vector>
#include <string>
#include <type_traits>
#include <limits>


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

	using byte_t = uint8_t;

	class byte_buffer {
		static_assert(CHAR_BIT == 8, "damn");
	private:
		std::vector<byte_t> m_data;

	public:
		inline byte_buffer() { }

		inline byte_buffer(const byte_t *data_, size_t sz) : m_data(data_, data_ + sz) { }

		inline size_t size() const {
			return m_data.size();
		}

		inline const byte_t * data() const {
			return &m_data[0];
		}

		inline const byte_t * dump(byte_t *out) const {
			std::memcpy(out, &m_data[0], m_data.size());
			return out;
		}

		template <typename IntT, typename Enable = typename std::enable_if<std::is_integral<IntT>::value, void>::type>
		inline void add_int(IntT d) {
			uintmax_t mask = uintmax_t(0xFF) << (8 * sizeof(IntT) - 8);
			for (unsigned i = 0; i < sizeof(IntT); i++) {
				uintmax_t md = mask & uintmax_t(d);
				md >>= (8 * (sizeof(IntT) - i - 1));
				add_int<byte_t>(md);
				mask >>= 8;
			}
		}

		template <>
		inline void add_int<byte_t>(byte_t d) {
			m_data.push_back(d);
		}

		template <typename IntT, typename Enable = typename std::enable_if<std::is_integral<IntT>::value, void>::type>
		inline void add_array(const IntT *d, size_t sz) {
			for (size_t i = 0; i < sz; i++) {
				add_int(d[i]);
			}
		}

		template <>
		inline void add_array<byte_t>(const byte_t *d, size_t sz) {
			m_data.insert(m_data.cend(), d, d + sz);
		}

		inline void add_string(const std::string &str) {
			assert(str.length() <= std::numeric_limits<uint16_t>::max());
			add_int<uint16_t>(str.length());
			add_array(&str[0], str.length());
		}

	};
}

#endif 