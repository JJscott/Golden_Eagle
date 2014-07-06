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
#include <stdexcept>


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

	using byte_t = unsigned char;

	// now kinda like an output stream
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
		inline void add(IntT d) {
			uintmax_t mask = uintmax_t(0xFF) << (8 * sizeof(IntT) - 8);
			for (unsigned i = 0; i < sizeof(IntT); i++) {
				uintmax_t md = mask & uintmax_t(d);
				md >>= (8 * (sizeof(IntT) - i - 1));
				m_data.push_back(byte_t(md));
				mask >>= 8;
			}
		}

		template <typename IntT, typename Enable = typename std::enable_if<std::is_integral<IntT>::value, void>::type>
		inline byte_buffer & operator<<(IntT d) {
			add(d);
			return *this;
		}

		template <typename IntT, typename Enable = typename std::enable_if<std::is_integral<IntT>::value, void>::type>
		inline void add_array(const IntT *d, size_t sz) {
			for (size_t i = 0; i < sz; i++) {
				add(d[i]);
			}
		}

		template <>
		inline void add_array<unsigned char>(const unsigned char *d, size_t sz) {
			m_data.insert(m_data.cend(), (byte_t *) d, (byte_t *) d + sz);
		}

		template <>
		inline void add_array<signed char>(const signed char *d, size_t sz) {
			m_data.insert(m_data.cend(), (byte_t *) d, (byte_t *) d + sz);
		}

		template <>
		inline void add_array<char>(const char *d, size_t sz) {
			m_data.insert(m_data.cend(), (byte_t *) d, (byte_t *) d + sz);
		}

		inline void add_string(const std::string &str) {
			assert(str.length() <= std::numeric_limits<uint16_t>::max());
			add<uint16_t>(str.length());
			add_array(&str[0], str.length());
		}

		inline byte_buffer & operator<<(const std::string &str) {
			add_string(str);
			return *this;
		}

		// something like an iterator / input stream
		class reader {
		private:
			const byte_buffer *m_buf;
			size_t m_i;

		public:
			inline explicit reader(const byte_buffer &buf_) : m_buf(&buf_), m_i(0) { }

			inline size_t size() const {
				return m_buf->size();
			}

			inline ptrdiff_t remaining(size_t i) const {
				return ptrdiff_t(m_buf->size()) - ptrdiff_t(i);
			}

			inline ptrdiff_t remaining() const {
				return remaining(m_i);
			}

			inline size_t position() const {
				return m_i;
			}

			inline void seek(size_t i) {
				m_i = i;
			}

			inline reader & operator+=(ptrdiff_t offset) {
				seek(size_t(ptrdiff_t(m_i) + offset));
			}

			inline reader & operator-=(ptrdiff_t offset) {
				seek(size_t(ptrdiff_t(m_i) - offset));
			}

			template <typename IntT, typename Enable = typename std::enable_if<std::is_integral<IntT>::value, void>::type>
			inline IntT peek(size_t i) const {
				if (remaining(i) < sizeof(IntT)) throw std::range_error("byte_buffer index out of range");
				uintmax_t ret = 0;
				for (unsigned j = 0; j < sizeof(IntT); j++) {
					uintmax_t b = m_buf->m_data[i + j];
					b <<= (8 * (sizeof(IntT) - j - 1));
					ret |= b;
				}
				return IntT(ret);
			}

			template <typename IntT, typename Enable = typename std::enable_if<std::is_integral<IntT>::value, void>::type>
			inline IntT peek() const {
				return peek<IntT>(m_i);
			}

			template <typename IntT, typename Enable = typename std::enable_if<std::is_integral<IntT>::value, void>::type>
			inline IntT get() {
				IntT d = peek<IntT>();
				seek(m_i + sizeof(IntT));
				return d;
			}

			template <typename IntT, typename Enable = typename std::enable_if<std::is_integral<IntT>::value, void>::type>
			inline reader & operator>>(IntT &d) {
				d = get<IntT>();
				return *this;
			}

			template <typename IntT, typename Enable = typename std::enable_if<std::is_integral<IntT>::value, void>::type>
			inline void peek_array(IntT *d, size_t sz, size_t i) const {
				if (remaining(i) < sz * sizeof(IntT)) throw std::range_error("byte_buffer index out of range");
				for (int j = 0; j < sz; j++) {
					*d++ = peek<IntT>(i + j);
				}
			}

			template <>
			inline void peek_array<unsigned char>(unsigned char *d, size_t sz, size_t i) const {
				if (remaining(i) < sz) throw std::range_error("byte_buffer index out of range");
				std::memcpy(d, &m_buf->m_data[i], sz);
			}

			template <>
			inline void peek_array<signed char>(signed char *d, size_t sz, size_t i) const {
				if (remaining(i) < sz) throw std::range_error("byte_buffer index out of range");
				std::memcpy(d, &m_buf->m_data[i], sz);
			}

			template <>
			inline void peek_array<char>(char *d, size_t sz, size_t i) const {
				if (remaining(i) < sz) throw std::range_error("byte_buffer index out of range");
				std::memcpy(d, &m_buf->m_data[i], sz);
			}

			template <typename IntT, typename Enable = typename std::enable_if<std::is_integral<IntT>::value, void>::type>
			inline void peek_array(IntT *d, size_t sz) const {
				peek_array<IntT>(d, sz, m_i);
			}

			template <typename IntT, typename Enable = typename std::enable_if<std::is_integral<IntT>::value, void>::type>
			inline void get_array(IntT *d, size_t sz) {
				peek_array<IntT>(d, sz);
				seek(m_i + sz * sizeof(IntT));
			}

			inline std::string peek_string(size_t i) const {
				unsigned len = peek<uint16_t>(i);
				if (remaining(i) < len) throw std::range_error("byte_buffer string length out of range");
				return std::string((const char *) &m_buf->m_data[i + 2], len);
			}

			inline std::string peek_string() const {
				return peek_string(m_i);
			}

			inline std::string get_string() {
				std::string str = peek_string();
				seek(m_i + 2 + str.length());
				return str;
			}

			inline reader & operator>>(std::string &str) {
				str = get_string();
				return *this;
			}

		};

		inline reader read() {
			return reader(*this);
		}

	};

}

#endif 