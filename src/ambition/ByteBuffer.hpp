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
#include <cstddef>
#include <vector>
#include <array>
#include <tuple>
#include <iostream>
#include <string>
#include <type_traits>
#include <limits>
#include <stdexcept>
#include <utility>

namespace ambition {
	using byte_t = unsigned char;

	// now kinda like an output stream
	class byte_buffer {
		static_assert(CHAR_BIT == 8, "damn");
	private:
		std::vector<byte_t> m_data;

		// test if fpu endianness matches cpu endianness, assuming iee754 representation or similar
		// http://stackoverflow.com/questions/2945174/floating-point-endianness
		static inline bool test_fpu_endianness() {
			static_assert(sizeof(float) == 4 && sizeof(double) == 8, "damn");
			// float with sign bit and exponent bit(s) set, but least significant mantissa bits 0
			float f = -1.0;
			uint32_t d = reinterpret_cast<uint32_t &>(f);
			bool mt = (d & 0x000000FF) == 0;
			bool mf = (d & 0xFF000000) == 0;
			assert((mt != mf) && "failed to determine host fpu endianness");
			return mt;
		}

	public:
		inline byte_buffer() { }

		inline byte_buffer(const byte_t *data_, size_t sz) : m_data(data_, data_ + sz) { }

		byte_buffer(const byte_buffer &) = default;
		byte_buffer & operator=(const byte_buffer &) = default;

		byte_buffer(byte_buffer &&other) : m_data(std::move(other.m_data)) { }

		byte_buffer & operator=(byte_buffer &&other) {
			m_data = std::move(other.m_data);
			return *this;
		}

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
		
		template <typename T>
		inline void add_array(const T *t, size_t sz) {
			for (size_t i = 0; i < sz; i++) {
				add(t[i]);
			}
		}
		
		template <typename T, typename Enable = void>
		struct add_impl {
			static const bool enable = false;
		};
		
		template <typename IntT>
		struct add_impl<IntT, typename std::enable_if<std::is_integral<IntT>::value, void>::type> {
			static const bool enable = true;
			
			inline static void go(byte_buffer &this_, IntT d) {
				uintmax_t mask = uintmax_t(0xFF) << (8 * sizeof(IntT) - 8);
				for (unsigned i = 0; i < sizeof(IntT); i++) {
					uintmax_t md = mask & uintmax_t(d);
					md >>= (8 * (sizeof(IntT) - i - 1));
					this_.m_data.push_back(byte_t(md));
					mask >>= 8;
				}
			}
		};
		
		template <typename Enable>
		struct add_impl<float, Enable> {
			static const bool enable = true;
			
			inline static void go(byte_buffer &this_, float f) {
				if (test_fpu_endianness()) {
					this_.add<uint32_t>(reinterpret_cast<uint32_t &>(f));
				} else {
					assert(false && "unimplemented");
				}
			}
		};
		
		template <typename Enable>
		struct add_impl<double, Enable> {
			static const bool enable = true;
			
			inline static void go(byte_buffer &this_, double f) {
				if (test_fpu_endianness()) {
					this_.add<uint64_t>(reinterpret_cast<uint64_t &>(f));
				} else {
					assert(false && "unimplemented");
				}
			}
		};
		
		template <typename Enable>
		struct add_impl<std::string, Enable> {
			static const bool enable = true;
			
			inline static void go(byte_buffer &this_, const std::string &str) {
				assert(str.length() <= std::numeric_limits<uint16_t>::max());
				this_.add<uint16_t>(str.length());
				this_.add_array<char>(&str[0], str.length());
			}
		};

		template <typename T>
		struct add_impl<std::vector<T>> {
			static const bool enable = true;

			inline static void go(byte_buffer &this_, const std::vector<T> &v) {
				assert(v.size() <= std::numeric_limits<uint16_t>::max());
				this_.add<uint16_t>(v.size());
				this_.add_array<T>(&v[0], v.size());
			}
		};

		template <typename T, size_t Size>
		struct add_impl<std::array<T, Size>> {
			static const bool enable = true;

			inline static void go(byte_buffer &this_, const std::array<T, Size> &a) {
				this_.add_array<T>(&a[0], Size);
			}
		};

		template <typename... TR>
		struct add_impl<std::tuple<TR...>> {
			static const bool enable = true;

			template <size_t Size, size_t I, typename TupleT>
			struct impl {
				inline static void go(byte_buffer &this_, const TupleT &t) {
					this_.add(std::get<I>(t));
					impl<Size, I + 1, TupleT>::go(this_, t);
				}
			};

			template <size_t Size, typename TupleT>
			struct impl<Size, Size, TupleT> {
				inline static void go(byte_buffer &this_, const TupleT &t) {
					// nothing - done.
				}
			};

			inline static void go(byte_buffer &this_, const std::tuple<TR...> &t) {
				using tuple_t = std::tuple<TR...>;
				impl<std::tuple_size<tuple_t>::value, 0, tuple_t>::go(this_, t);
			}
		};
		
		template <typename T, typename Enable = typename std::enable_if<add_impl<T>::enable, void>::type>
		inline void add(const T &t) {
			add_impl<T>::go(*this, t);
		}
		
		template <typename CharT, typename Enable = typename std::enable_if<std::is_integral<CharT>::value && sizeof(CharT) == 1>::type>
		inline void add(const CharT *p) {
			add(std::string(p));
		}
		
		template <typename T, typename Enable = typename std::enable_if<add_impl<T>::enable, void>::type>
		inline byte_buffer & operator<<(const T &t) {
			add<T>(t);
			return *this;
		}
		
		template <typename CharT, typename Enable = typename std::enable_if<std::is_integral<CharT>::value && sizeof(CharT) == 1>::type>
		inline byte_buffer & operator<<(const CharT *p) {
			add(std::string(p));
			return *this;
		}
		
		// something like an input stream (slightly iterator-ish)
		class reader {
		private:
			const byte_buffer *m_buf;
			size_t m_i;

		public:
			inline explicit reader(const byte_buffer &buf_) : m_buf(&buf_), m_i(0) { }

			inline size_t size() const {
				return m_buf->size();
			}

			inline std::ptrdiff_t remaining(size_t i) const {
				return std::ptrdiff_t(m_buf->size()) - std::ptrdiff_t(i);
			}

			inline std::ptrdiff_t remaining() const {
				return remaining(m_i);
			}

			inline size_t position() const {
				return m_i;
			}

			inline void seek(size_t i) {
				m_i = i;
			}

			inline reader & operator+=(std::ptrdiff_t offset) {
				seek(size_t(ptrdiff_t(m_i) + offset));
				return *this;
			}

			inline reader & operator-=(std::ptrdiff_t offset) {
				seek(size_t(ptrdiff_t(m_i) - offset));
				return *this;
			}

			template <typename T>
			inline size_t peek_array(T *t, size_t sz, size_t i) const {
				size_t c = 0;
				for (int j = 0; j < sz; j++) {
					c += peek_impl<T>::go(*this, i + c, *t++);
				}
				return c;
			}

			template <typename T>
			inline size_t peek_array(T *t, size_t sz) const {
				return peek_array<T>(t, sz, m_i);
			}
			
			template <typename T>
			inline void get_array(T *t, size_t sz) {
				size_t c = peek_array<T>(t, sz);
				seek(m_i + c);
			}
			
			template <typename T, typename Enable = void>
			struct peek_impl {
				static const bool enable = false;
			};
			
			template <typename IntT>
			struct peek_impl<IntT, typename std::enable_if<std::is_integral<IntT>::value, void>::type> {
				static const bool enable = true;
				
				inline static size_t go(const reader &this_, size_t i, IntT &d) {
					if (this_.remaining(i) < std::ptrdiff_t(sizeof(IntT))) throw std::range_error("byte_buffer index out of range");
					uintmax_t ret = 0;
					for (unsigned j = 0; j < sizeof(IntT); j++) {
						uintmax_t b = this_.m_buf->m_data[i + j];
						b <<= (8 * (sizeof(IntT) - j - 1));
						ret |= b;
					}
					d = IntT(ret);
					return sizeof(IntT);
				}
			};
			
			template <typename Enable>
			struct peek_impl<float, Enable> {
				static const bool enable = true;
				
				inline static size_t go(const reader &this_, size_t i, float &f) {
					if (this_.remaining(i) < std::ptrdiff_t(sizeof(float))) throw std::range_error("byte_buffer index out of range");
					uint32_t d = this_.peek<uint32_t>(i);
					if (test_fpu_endianness()) {
						f = reinterpret_cast<float &>(d);
					} else {
						assert(false && "not implemented");
						f = 0;
					}
					return sizeof(float);
				}
			};
			
			template <typename Enable>
			struct peek_impl<double, Enable> {
				static const bool enable = true;
				
				inline static size_t go(const reader &this_, size_t i, double &f) {
					if (this_.remaining(i) < std::ptrdiff_t(sizeof(double))) throw std::range_error("byte_buffer index out of range");
					uint64_t d = this_.peek<uint64_t>(i);
					if (test_fpu_endianness()) {
						f = reinterpret_cast<double &>(d);
					} else {
						assert(false && "not implemented");
						f = 0;
					}
					return sizeof(double);
				}
			};

			template <typename Enable>
			struct peek_impl<std::string, Enable> {
				static const bool enable = true;
				
				inline static size_t go(const reader &this_, size_t i, std::string &str) {
					uint16_t len;
					size_t c = peek_impl<uint16_t>::go(this_, i, len);
					if (this_.remaining(i + c) < std::ptrdiff_t(len)) throw std::range_error("byte_buffer string length out of range");
					str = std::string(reinterpret_cast<const char *>(&this_.m_buf->m_data[i + c]), len);
					c += size_t(len);
					return c;
				}
			};

			template <typename T>
			struct peek_impl<std::vector<T>> {
				static const bool enable = true;

				inline static size_t go(const reader &this_, size_t i, std::vector<T> &v) {
					uint16_t len;
					size_t c = peek_impl<uint16_t>::go(this_, i, len);
					v = std::vector<T>(len);
					c += this_.peek_array<T>(&v[0], len, i + c);
					return c;
				}
			};

			template <typename T, size_t Size>
			struct peek_impl<std::array<T, Size>> {
				static const bool enable = true;

				inline static size_t go(const reader &this_, size_t i, std::array<T, Size> &a) {
					return this_.peek_array<T>(&a[0], Size, i);
				}
			};

			template <typename... TR>
			struct peek_impl<std::tuple<TR...>> {
				static const bool enable = true;

				template <size_t Size, size_t I, typename TupleT>
				struct impl {
					inline static size_t go(const reader &this_, size_t i, TupleT &t) {
						size_t c = peek_impl<typename std::tuple_element<I, TupleT>::type>::go(this_, i, std::get<I>(t));
						c += impl<Size, I + 1, TupleT>::go(this_, i + c, t);
						return c;
					}
				};

				template <size_t Size, typename TupleT>
				struct impl<Size, Size, TupleT> {
					inline static size_t go(const reader &this_, size_t i, TupleT &t) {
						// nothing - done.
						return 0;
					}
				};

				inline static size_t go(const reader &this_, size_t i, std::tuple<TR...> &t) {
					using tuple_t = std::tuple<TR...>;
					return impl<std::tuple_size<tuple_t>::value, 0, tuple_t>::go(this_, i, t);
				}
			};

			template <typename T, typename Enable = typename std::enable_if<peek_impl<T>::enable, void>::type>
			inline T peek(size_t i) const {
				T t;
				peek_impl<T>::go(*this, i, t);
				return t;
			}
			
			template <typename T, typename Enable = typename std::enable_if<peek_impl<T>::enable, void>::type>
			inline T peek() const {
				return peek<T>(m_i);
			}
			
			template <typename T, typename Enable = typename std::enable_if<peek_impl<T>::enable, void>::type>
			inline T get() {
				T t;
				size_t c = peek_impl<T>::go(*this, m_i, t);
				seek(m_i + c);
				return t;
			}
			
			template <typename T, typename Enable = typename std::enable_if<peek_impl<T>::enable, void>::type>
			inline reader & operator>>(T &t) {
				t = get<T>();
				return *this;
			}

		};

		inline reader read() const {
			return reader(*this);
		}

	};
	
	template <>
	inline void byte_buffer::add_array<unsigned char>(const unsigned char *d, size_t sz) {
		m_data.insert<const byte_t *>(m_data.end(), reinterpret_cast<const byte_t *>(d), reinterpret_cast<const byte_t *>(d) + sz);
	}

	template <>
	inline void byte_buffer::add_array<signed char>(const signed char *d, size_t sz) {
		m_data.insert<const byte_t *>(m_data.end(), reinterpret_cast<const byte_t *>(d), reinterpret_cast<const byte_t *>(d) + sz);
	}

	template <>
	inline void byte_buffer::add_array<char>(const char *d, size_t sz) {
		m_data.insert<const byte_t *>(m_data.end(), reinterpret_cast<const byte_t *>(d), reinterpret_cast<const byte_t *>(d) + sz);
	}
	
	template <>
	inline size_t byte_buffer::reader::peek_array<unsigned char>(unsigned char *d, size_t sz, size_t i) const {
		if (remaining(i) < std::ptrdiff_t(sz)) throw std::range_error("byte_buffer index out of range");
		std::memcpy(d, &m_buf->m_data[i], sz);
		return sz;
	}

	template <>
	inline size_t byte_buffer::reader::peek_array<signed char>(signed char *d, size_t sz, size_t i) const {
		if (remaining(i) < std::ptrdiff_t(sz)) throw std::range_error("byte_buffer index out of range");
		std::memcpy(d, &m_buf->m_data[i], sz);
		return sz;
	}

	template <>
	inline size_t byte_buffer::reader::peek_array<char>(char *d, size_t sz, size_t i) const {
		if (remaining(i) < std::ptrdiff_t(sz)) throw std::range_error("byte_buffer index out of range");
		std::memcpy(d, &m_buf->m_data[i], sz);
		return sz;
	}

}

#endif 
