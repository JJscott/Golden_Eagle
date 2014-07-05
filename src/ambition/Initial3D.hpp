/*
 *
 * Initial3D Main Header
 * 
 * This is a header-only library, or at least that's what im trying to do.
 * Asserts are used, so make sure NDEBUG is defined consistently across compilation units.
 * 
 * Mainly math stuff atm.
 * 
 * Still a W.I.P.
 * 
 * @author Ben Allen
 * 
 * TODO
 * - quat / vector interpolation
 * - vec3, vec4, quat, mat4 overloads / specialisations of math:: functions
 * - complex stuff?
 * - gamma, erf, etc?
 * - numerical calculus?
 *
 */

#ifndef INITIAL3D_HPP
#define INITIAL3D_HPP

#include <cstdlib>
#include <cassert>
#include <cmath>
#include <ctime>
#include <climits>
#include <complex>
#include <limits>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

// **** macros...
#undef MIN
#undef min
#undef MAX
#undef max

namespace initial3d {
	
	//
	// Type Declarations
	//
	
	// exception thrown when vec3, vec4 or quat constructor sets a member to nan.
	class nan_error;
	
	template <typename> class vec3;
	typedef vec3<double> vec3d;
	typedef vec3<float> vec3f;
	
	template <typename> class vec4;
	typedef vec4<double> vec4d;
	typedef vec4<float> vec4f;
	
	template <typename> class quat;
	typedef quat<double> quatd;
	typedef quat<float> quatf;
	
	template <typename> class mat4;
	typedef mat4<double> mat4d;
	typedef mat4<float> mat4f;
	
	// inherit (privately) from this to make a class not copy constructible or assignable.
	class Uncopyable {
	private:
		Uncopyable(const Uncopyable &);
		Uncopyable & operator=(const Uncopyable &);
	protected:
		Uncopyable() { }
	};
	
	namespace math {
		// template functions with default implementations
		// specialise to refine behaviour of other types
		// minvalue, maxvalue, inf, isinf, nan delegate to std::numeric_limits
		// isnan tests 'a != a'
		
		// min specialisation for full functionality (these default to std implementations):
		// abs, floor, ceil, pow, exp, log, log10, sqrt
		// sin, cos, tan, asin, acos, atan, atan2, sinh, cosh, tanh
		
		template <typename T> inline T min(const T &a, const T &b) {
			return a < b ? a : b;
		}
		
		template <typename T> inline T max(const T &a, const T &b) {
			return b < a ? a : b;
		}
		
		template <typename T> inline T clamp(const T &a, const T &lower, const T &upper) {
			return (a < lower) ? lower : ((upper < a) ? upper : a);
		}
		
		template <typename T, typename U>
		inline T lerp(const T &x0, const T &x1, const U &t) {
			return x0 * (1 - t) + x1 * t;
		}

		template <typename T> inline T real(const std::complex<T> &a) {
			return std::real(a);
		}
		
		template <typename T> inline T imag(const std::complex<T> &a) {
			return std::imag(a);
		}
		
		template <typename T> inline T abs(const T &a) {
			return a < 0 ? -a : a;
		}
		
		template <> inline double abs(const double &a) {
			return std::abs(a);
		}
		
		template <> inline float abs(const float &a) {
			return std::abs(a);
		}
		
		template <typename T> inline T abs(const std::complex<T> &a) {
			return std::abs(a);
		}
		
		template <typename T> inline T arg(const std::complex<T> &a) {
			return std::arg(a);
		}
		
		template <typename T> inline std::complex<T> conj(const std::complex<T> &a) {
			return std::conj(a);
		}
		
		template <typename T> inline T floor(const T &a) {
			return std::floor(a);
		}
		
		template <typename T> inline T ceil(const T &a) {
			return std::ceil(a);
		}
		
		template <typename T> inline int signum(const T &a) {
			return (a < 0) ? -1 : ((0 < a) ? 1 : 0);
		}
		
		template <typename T> inline T copysign(const T &mag, const T &sign) {
			return abs(mag) * signum(sign);
		}
		
		template <typename T> inline T minvalue() {
			return std::numeric_limits<T>::min();
		}
		
		template <typename T> inline T maxvalue() {
			return std::numeric_limits<T>::max();
		}
		
		template <typename T> inline T inf() {
			return std::numeric_limits<T>::infinity();
		}
		
		template <typename T> inline bool isinf(const T &a) {
			return std::numeric_limits<T>::max() < abs(a);
		}
		
		template <typename T> inline T nan() {
			return std::numeric_limits<T>::quiet_NaN();
		}
		
		template <typename T> inline bool isnan(const T &a) {
			// FIXME does this require ieee754 fpu?
			return a != a;
		}
		
		template <typename T, typename U> inline T pow(const T &a, const U &power) {
			return std::pow(a, power);
		}
		
		template <typename T> inline T exp(const T &a) {
			return std::exp(a);
		}
		
		template <typename T> inline T exp10(const T &a) {
			return pow(10.0, a);
		}
		
		template <typename T> inline T exp2(const T &a) {
			return pow(2.0, a);
		}
		
		template <typename T> inline T log(const T &a) {
			return std::log(a);
		}
		
		template <typename T> inline T log10(const T &a) {
			return std::log10(a);
		}
		
		template <typename T> inline T log2(const T &a) {
			return log(a) * 1.4426950408889634073599246810019;
		}
		
		template <typename T, typename U> inline T log(const T &a, const U &base) {
			return log(a) / log(base);
		}
		
		template <typename T> inline T sq(const T &a) {
			return a * a;
		}
		
		template <typename T> inline T sqrt(const T &a) {
			return std::sqrt(a);
		}
		
		template <typename T> inline T cb(const T &a) {
			return a * a * a;
		}
		
		template <typename T> inline T cbrt(const T &a) {
			return pow(a, 1.0 / 3.0);
		}
		
		template <typename T> inline T hypot(const T &x, const T &y) {
			return sqrt(sq(x) + sq(y));
		}
		
		template <typename T> inline T hypot(const T &x, const T &y, const T &z) {
			return sqrt(sq(x) + sq(y) + sq(z));
		}
		
		template <typename T> inline T hypot(const T &x, const T &y, const T &z, const T &w) {
			return sqrt(sq(x) + sq(y) + sq(z) + sq(w));
		}
		
		template <typename T> inline T sin(const T &a) {
			return std::sin(a);
		}
		
		template <typename T> inline T cos(const T &a) {
			return std::cos(a);
		}
		
		template <typename T> inline T tan(const T &a) {
			return std::tan(a);
		}
		
		template <typename T> inline T cot(const T &a) {
			return cos(a) / sin(a);
		}
		
		template <typename T> inline T sec(const T &a) {
			return 1.0 / cos(a);
		}
		
		template <typename T> inline T csc(const T &a) {
			return 1.0 / sin(a);
		}
		
		template <typename T> inline T asin(const T &a) {
			return std::asin(a);
		}
		
		template <typename T> inline T acos(const T &a) {
			return std::acos(a);
		}
		
		template <typename T> inline T atan(const T &a) {
			return std::atan(a);
		}
		
		template <typename T> inline T atan2(const T &y, const T &x) {
			return std::atan2(y, x);
		}
		
		template <typename T> inline T acot(const T &a) {
			return 1.5707963267948966192313216916398 - atan(a);
		}
		
		template <typename T> inline T asec(const T &a) {
			return acos(1 / a);
		}
		
		template <typename T> inline T acsc(const T &a) {
			return asin(1 / a);
		}
		
		template <typename T> inline T sinh(const T &a) {
			return std::sinh(a);
		}
		
		template <typename T> inline T cosh(const T &a) {
			return std::cosh(a);
		}
		
		template <typename T> inline T tanh(const T &a) {
			return std::tanh(a);
		}
		
		template <typename T> inline T coth(const T &a) {
			return cosh(a) / sinh(a);
		}
		
		template <typename T> inline T sech(const T &a) {
			return 1.0 / cosh(a);
		}
		
		template <typename T> inline T csch(const T &a) {
			return 1.0 / sinh(a);
		}
		
		template <typename T> inline T asinh(const T &a) {
			return log(a + sqrt(sq(a) + 1));
		}
		
		template <typename T> inline T acosh(const T &a) {
			return log(a + sqrt(sq(a) - 1));
		}
		
		template <typename T> inline T atanh(const T &a) {
			return 0.5 * log((1 + a) / (1 - a));
		}
		
		template <typename T> inline T acoth(const T &a) {
			return 0.5 * log((a + 1) / (a - 1));
		}
		
		template <typename T> inline T asech(const T &a) {
			return log(1 / a + sqrt(1 - sq(a)) / a);
		}
		
		template <typename T> inline T acsch(const T &a) {
			return log(1 / a + sqrt(1 + sq(a)) / abs(a));
		}
		
		template <typename T> inline std::complex<T> cis(const T &theta) {
			return std::complex<T>(cos(theta), sin(theta));
		}
		
		// pi
		inline double pi() {
			return 3.1415926535897932384626433832795;
		}
		
		// natural log base
		inline double e() {
			return 2.7182818284590452353602874713527;
		}
		
		// golden ratio
		inline double phi() {
			return 1.61803398874989484820458683436563811;
		}
		
	}
	
	class nan_error : public std::runtime_error {
		public:
		explicit inline nan_error(const std::string & what_ = "NaN bug intercepted!") : 
			std::runtime_error(what_) { }
	};
	
	template <typename T> inline void checknan(const T & t) {
		if (math::isnan(t)) throw nan_error();
	}
	
	// Provides assignable access to elements whose container must update other
	// data when they are assigned.
	template <typename T, typename U>
	class accessor {
	private:
		T *m_t;
		U *m_u;
		U (*m_getter)(const T *, U *);
		void (*m_setter)(T *, U *, const U &);
		
	public:
		// container, element (pointer, so null can be used), get-function, set-function
		inline accessor(T *t_, U *u_, U (*getter_)(const T *, U *), void (*setter_)(T *, U *, const U &)) : 
			m_t(t_), m_u(u_), m_getter(getter_), m_setter(setter_) { }
		
		// assignment
		inline accessor<T, U> & operator=(const U & rhs) {
			m_setter(m_t, m_u, rhs);
			return *this;
		}

		// add-assign
		inline accessor<T, U> & operator+=(const U & rhs) {
			U u = m_getter(m_t, m_u) + rhs;
			m_setter(m_t, m_u, u);
			return *this;
		}
		
		// subtract-assign
		inline accessor<T, U> & operator-=(const U & rhs) {
			U u = m_getter(m_t, m_u) - rhs;
			m_setter(m_t, m_u, u);
			return *this;
		}

		// multiply-assign
		inline accessor<T, U> & operator*=(const U & rhs) {
			U u = m_getter(m_t, m_u) * rhs;
			m_setter(m_t, m_u, u);
			return *this;
		}

		// divide-assign
		inline accessor<T, U> & operator/=(const U & rhs) {
			U u = m_getter(m_t, m_u) / rhs;
			m_setter(m_t, m_u, u);
			return *this;
		}

		// cast to component type
		inline operator U() const {
			return m_getter(m_t, m_u);
		}
		
		// stream insertion
		inline friend std::ostream & operator<<(std::ostream & out, const accessor<T, U> & a) {
			return out << static_cast<U>(a);
		}
		
		// stream extraction
		inline friend std::istream & operator>>(std::istream & in, accessor<T, U> a) {
			// the accessor arg is not taken by reference to allow for it being an rvalue
			U u;
			in >> u;
			a = u;
			return in;
		}
	};
	
	//
	// 3-element vector
	//
	template <typename T>
	class vec3 {
		
	private:
		T m_x, m_y, m_z;
		mutable T m_mag;
		
		inline void checkMag() const {
			if (m_mag < 0) m_mag = math::sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
		}
		
		inline static T getElement(const vec3<T> *v, T *el) {
			return *el;
		}
		
		inline static void setElement(vec3<T> *v, T *el, const T &newval) {
			*el = newval;
			v->m_mag = -1;
		}
		
		inline static T getMag(const vec3<T> *v, T *el) {
			v->checkMag();
			return v->m_mag;
		}
		
		inline static void setMag(vec3<T> *v, T *el, const T &newval) {
			v->checkMag();
			*v *= newval / v->m_mag;
		}
		
	public:
		typedef T element_t;
		
		inline T x() const {
			return m_x;
		}
		
		inline T y() const {
			return m_y;
		}
		
		inline T z() const {
			return m_z;
		}
		
		//inline T operator[](size_t i) const {
		//	assert(i < 3);
		//	return *(&m_x + i);
		//}

		// implicit pointer conversion
		inline operator T const *() const {
			return &m_x;
		}
		
		inline T mag() const {
			checkMag();
			return m_mag;
		}
		
		// using the default copy constructor and assignment
		
		// default constructor, initialises to zero
		inline vec3() :
			m_x(0), m_y(0), m_z(0), m_mag(0) { }
		
		// element-wise constructor
		inline vec3(const T & x_, const T & y_, const T & z_) :
			m_x(x_), m_y(y_), m_z(z_), m_mag(-1) {
				checknan(m_x + m_y + m_z);
			}
		
		// cross-component-type copy constructor
		template <typename U>
		inline vec3(const vec3<U> other_) :
			m_x(static_cast<T>(other_.x())),
			m_y(static_cast<T>(other_.y())),
			m_z(static_cast<T>(other_.z())),
			m_mag(-1) {
				checknan(m_x + m_y + m_z);
			}
			
		inline accessor<vec3<T>, T> x() {
			return accessor<vec3<T>, T>(this, &m_x, getElement, setElement);
		}
		
		inline accessor<vec3<T>, T> y() {
			return accessor<vec3<T>, T>(this, &m_y, getElement, setElement);
		}
		
		inline accessor<vec3<T>, T> z() {
			return accessor<vec3<T>, T>(this, &m_z, getElement, setElement);
		}
		
		//inline accessor<vec3<T>, T> operator[](size_t i) {
		//	assert(i < 3);
		//	return accessor<vec3<T>, T>(this, &m_x + i, getElement, setElement);
		//}
		
		inline accessor<vec3<T>, T> mag() {
			return accessor<vec3<T>, T>(this, NULL, getMag, setMag);
		}
		
		inline vec3<T> add(const vec3<T> &rhs) const {
			return vec3<T>(m_x + rhs.m_x, m_y + rhs.m_y, m_z + rhs.m_z);
		}
		
		inline vec3<T> subtract(const vec3<T> &rhs) const {
			return vec3<T>(m_x - rhs.m_x, m_y - rhs.m_y, m_z - rhs.m_z);
		}
		
		inline vec3<T> negate() const {
			vec3<T> v(-m_x, -m_y, -m_z);
			v.m_mag = m_mag;
			return v;
		}
		
		inline T dot(const vec3<T> &rhs) const {
			return m_x * rhs.m_x + m_y * rhs.m_y + m_z * rhs.m_z;
		}
		
		inline vec3<T> cross(const vec3<T> &rhs) const {
			return vec3<T>(m_y * rhs.m_z - m_z * rhs.m_y, m_z * rhs.m_x - m_x * rhs.m_z, m_x * rhs.m_y - m_y * rhs.m_x);
		}
		
		inline vec3<T> multiply(const T &f) const {
			vec3<T> v(m_x * f, m_y * f, m_z * f);
			if (m_mag >= 0) v.m_mag = m_mag * f;
			return v;
		}
		
		inline vec3<T> unit() const {
			vec3<T> v = multiply(1 / mag());
			v.m_mag = 1;
			return v;
		}
		
		inline T distance(const vec3<T> &v) const {
			T dx = m_x - v.m_x;
			T dy = m_y - v.m_y;
			T dz = m_z - v.m_z;
			return math::sqrt(dx * dx + dy * dy + dz * dz);
		}
		
		inline T angle(const vec3<T> &v) const {
			return math::acos(dot(v) / (mag() * v.mag()));
		}
		
		inline vec3<T> project(const vec3<T> &v) const {
			return v.multiply(dot(v) / v.dot(v));
		}
		
		inline vec3<T> reject(const vec3<T> &v) const {
			return subtract(v.multiply(dot(v) / v.dot(v)));
		}
		
		// magnitude
		inline T operator+() const {
			return mag();
		}
		
		// assignable magnitude
		inline accessor<vec3<T>, T> operator+() {
			return mag();
		}
		
		// unit
		inline vec3<T> operator~() const {
			return unit();
		}
		
		// add
		inline vec3<T> operator+(const vec3<T> & rhs) const {
			return add(rhs);
		}
		
		// add-assign
		inline vec3<T> & operator+=(const vec3<T> & rhs) {
			return *this = add(rhs);
		}
		
		// subtract
		inline vec3<T> operator-(const vec3<T> & rhs) const {
			return subtract(rhs);
		}
		
		// subtract-assign
		inline vec3<T> & operator-=(const vec3<T> & rhs) {
			return *this = subtract(rhs);
		}
		
		// negate
		inline vec3<T> operator-() const {
			return negate();
		}
		
		// dot
		inline T operator*(const vec3<T> & rhs) const {
			return dot(rhs);
		}
		
		// cross
		inline vec3<T> operator^(const vec3<T> & rhs) const {
			return cross(rhs);
		}
		
		// cross-assign
		inline vec3<T> & operator^=(const vec3<T> & rhs) {
			return *this = cross(rhs);
		}
		
		// scalar multiply
		inline vec3<T> operator*(const T & rhs) const {
			return multiply(rhs);
		}
		
		// scalar multiply-assign
		inline vec3<T> & operator*=(const T & rhs) {
			return *this = multiply(rhs);
		}
		
		// scale, other order
		inline friend vec3<T> operator*(const T & lhs, const vec3<T> & rhs) {
			return rhs.multiply(lhs);
		}
		
		// scalar divide
		inline vec3<T> operator/(const T & rhs) const {
			return multiply(1 / rhs);
		}
		
		// scalar divide-assign
		inline vec3<T> & operator/=(const T & rhs) {
			return *this = multiply(1 / rhs);
		}
		
		// stream insertion
		inline friend std::ostream & operator<<(std::ostream & out, const vec3<T> & v) {
			return out << '(' << v.x() << ", " << v.y() << ", " << v.z() << ')';
		}
		
		inline static vec3<T> zero() {
			return vec3<T>();
		}
		
		inline static vec3<T> one() {
			return vec3<T>(1, 1, 1);
		}
		
		inline static vec3<T> i(const T & t) {
			return vec3<T>(t, 0, 0);
		}
		
		inline static vec3<T> i() {
			return vec3<T>::i(1);
		}
		
		inline static vec3<T> j(const T & t) {
			return vec3<T>(0, t, 0);
		}
		
		inline static vec3<T> j() {
			return vec3<T>::j(1);
		}
		
		inline static vec3<T> k(const T & t) {
			return vec3<T>(0, 0, t);
		}
		
		inline static vec3<T> k() {
			return vec3<T>::k(1);
		}
		
		inline static vec3<T> positive_extremes(const vec3<T> & v0, const vec3<T> & v1) {
			return vec3<T>(math::max(v0.x(), v1.x()), math::max(v0.y(), v1.y()), math::max(v0.z(), v1.z()));
		}
		
		inline static vec3<T> negative_extremes(const vec3<T> & v0, const vec3<T> & v1) {
			return vec3<T>(math::min(v0.x(), v1.x()), math::min(v0.y(), v1.y()), math::min(v0.z(), v1.z()));
		}
		
		inline static vec3<T> plane_norm(const vec3<T> & v0, const vec3<T> & v1, const vec3<T> & v2) {
			return ~((v1 - v0) ^ (v2 - v1));
		}
		
	};
	
	//
	// 4-element vector
	//
	template <typename T>
	class vec4 {
		
	private:
		T m_x, m_y, m_z, m_w;
		
	public:
		typedef T element_t;
		
		inline T x() const {
			return m_x;
		}
		
		inline T y() const {
			return m_y;
		}
		
		inline T z() const {
			return m_z;
		}
		
		inline T w() const {
			return m_w;
		}
		
		//inline T operator[](size_t i) const {
		//	assert(i < 4);
		//	return *(&m_x + i);
		//}

		// implicit pointer conversion
		inline operator T const *() const {
			return &m_x;
		}
		
		// using the default copy constructor and assignment
		
		// default constructor, initialises to zero
		inline vec4() :
			m_x(0), m_y(0), m_z(0), m_w(0) { }
		
		// element-wise constructor
		inline vec4(const T & x_, const T & y_, const T & z_, const T & w_) :
			m_x(x_), m_y(y_), m_z(z_), m_w(w_) {
				checknan(m_x + m_y + m_z + m_w);
			}
		
		// cross-component-type copy constructor
		template <typename U>
		inline vec4(const vec4<U> & other_) :
			m_x(static_cast<T>(other_.x())),
			m_y(static_cast<T>(other_.y())),
			m_z(static_cast<T>(other_.z())),
			m_w(static_cast<T>(other_.w())) {
				checknan(m_x + m_y + m_z + m_w);
			}
		
		// cross-component-type implicit conversion constructor
		template <typename U>
		inline vec4(const vec3<U> & other_) :
			m_x(static_cast<T>(other_.x())),
			m_y(static_cast<T>(other_.y())),
			m_z(static_cast<T>(other_.z())),
			m_w(1) {
				checknan(m_x + m_y + m_z + m_w);
			}
		
		// cross-component-type conversion constructor
		template <typename U>
		inline vec4(const vec3<U> & other_, const T & w_) :
			m_x(static_cast<T>(other_.x())),
			m_y(static_cast<T>(other_.y())),
			m_z(static_cast<T>(other_.z())),
			m_w(w_) {
				checknan(m_x + m_y + m_z + m_w);
			}
		
		inline T & x() {
			return m_x;
		}
		
		inline T & y() {
			return m_y;
		}
		
		inline T & z() {
			return m_z;
		}
		
		inline T & w() {
			return m_w;
		}
		
		//inline T & operator[](size_t i) {
		//	assert(i < 4);
		//	return *(&m_x + i);
		//}
		
		inline vec4<T> homogenise() const {
			T iw = 1 / m_w;
			return vec4<T>(m_x * iw, m_y * iw, m_z * iw, 1);
		}
		
		// cross-component-type non-homogenising vec3 conversion
		template <typename U>
		inline vec3<U> xyz() const {
			return vec3<U>(m_x, m_y, m_z);
		}
		
		// cross-component-type implicit homogenising vec3 cast
		template <typename U>
		inline operator vec3<U>() const {
			return homogenise().template xyz<U>();
		}
		
		inline vec4<T> add(const vec4<T> & rhs) const {
			return vec4<T>(m_x + rhs.x(), m_y + rhs.y(), m_z + rhs.z(), m_w + rhs.w());
		}
		
		inline vec4<T> subtract(const vec4<T> & rhs) const {
			return vec4<T>(m_x - rhs.x(), m_y - rhs.y(), m_z - rhs.z(), m_w - rhs.w());
		}
		
		inline vec4<T> negate() const {
			return vec4<T>(-m_x, -m_y, -m_z, -m_w);
		}
		
		inline T dot(const vec4<T> & rhs) const {
			return m_x * rhs.x() + m_y * rhs.y() + m_z * rhs.z() + m_w * rhs.w();
		}
		
		inline vec4<T> multiply(const T & rhs) const {
			return vec4<T>(m_x * rhs, m_y * rhs, m_z * rhs, m_w * rhs);
		}
		
		// add
		inline vec4<T> operator+(const vec4<T> & rhs) const {
			return add(rhs);
		}
		
		// add-assign
		inline vec4<T> & operator+=(const vec4<T> & rhs) {
			return *this = add(rhs);
		}
		
		// subtract
		inline vec4<T> operator-(const vec4<T> & rhs) const {
			return subtract(rhs);
		}
		
		// subtract-assign
		inline vec4<T> & operator-=(const vec4<T> & rhs) {
			return *this = subtract(rhs);
		}
		
		// negate
		inline vec4<T> operator-() const {
			return negate();
		}
		
		// dot
		inline T operator*(const vec4<T> & rhs) const {
			return dot(rhs);
		}
		
		// scalar multiply
		inline vec4<T> operator*(const T & rhs) const {
			return multiply(rhs);
		}
		
		// scalar multiply-assign
		inline vec4<T> & operator*=(const T & rhs) {
			return *this = multiply(rhs);
		}
		
		// scale, other order
		inline friend vec4<T> operator*(const T & lhs, const vec4<T> & rhs) {
			return rhs.multiply(lhs);
		}
		
		// scalar divide
		inline vec4<T> operator/(const T & rhs) const {
			return multiply(1 / rhs);
		}
		
		// scalar divide-assign
		inline vec4<T> & operator/=(const T & rhs) {
			return *this = multiply(1 / rhs);
		}
		
		// stream insertion
		inline friend std::ostream & operator<<(std::ostream & out, const vec4<T> & v) {
			return out << '(' << v.x() << ", " << v.y() << ", " << v.z() << ", " << v.w() << ')';
		}
		
	};
	
	//
	// quaternion
	//
	template <typename T>
	class quat {
	
	private:
		T m_w, m_x, m_y, m_z;
	
	public:
		typedef T element_t;
		
		inline T w() const {
			return m_w;
		}
		
		inline T x() const {
			return m_x;
		}
		
		inline T y() const {
			return m_y;
		}
		
		inline T z() const {
			return m_z;
		}
		
		//inline T operator[](size_t i) const {
		//	assert(i < 4);
		//	return *(&m_w + i);
		//}
		
		// using default copy constructor and assignment
		
		// default constructor, initialises to one
		inline quat() :
			m_w(1), m_x(0), m_y(0), m_z(0) { }
		
		// element-wise constructor
		inline quat(const T & w_, const T & x_, const T & y_, const T & z_) :
			m_w(w_), m_x(x_), m_y(y_), m_z(z_) {
				checknan(m_w + m_x + m_y + m_z);
			}
		
		// cross-component-type copy constructor
		template <typename U>
		inline quat(const quat<U> & other_) :
			m_w(static_cast<T>(other_.w())),
			m_x(static_cast<T>(other_.x())),
			m_y(static_cast<T>(other_.y())),
			m_z(static_cast<T>(other_.z())) {
				checknan(m_w + m_x + m_y + m_z);
			}
		
		// cross-component-type conversion constructor
		template <typename U>
		inline quat(const T & w_, const vec3<U> & xyz_) :
			m_w(w_),
			m_x(static_cast<T>(xyz_.x())),
			m_y(static_cast<T>(xyz_.y())),
			m_z(static_cast<T>(xyz_.z())) {
				checknan(m_w + m_x + m_y + m_z);
			}
		
		// axis-angle conversion
		inline static quat<T> axisangle(const vec3<T> & axis, const T & angle) {
			T w, x, y, z;
			// angle(+-2*PI) <=> w(-1) and angle(0) <=> w(1)
			vec3<T> axis_u = ~axis;
			T sin_a = math::sin(angle * 0.5);
			w = math::cos(angle * 0.5);
			x = axis_u.x() * sin_a;
			y = axis_u.y() * sin_a;
			z = axis_u.z() * sin_a;
			return quat<T>(w, x, y, z);
		}
		
		// rotation vector conversion (magnitude = angle)
		inline static quat<T> axisangle(const vec3<T> & rot) {
			T w, x, y, z;
			if (math::isinf(1 / +rot)) {
				w = 1;
				x = 0;
				y = 0;
				z = 0;
			} else {
				T angle = +rot;
				vec3<T> axis_u = ~rot;
				T sin_a = math::sin(angle * 0.5);
				w = math::cos(angle * 0.5);
				x = axis_u.x() * sin_a;
				y = axis_u.y() * sin_a;
				z = axis_u.z() * sin_a;
			}
			return quat<T>(w, x, y, z);
		}
		
		inline T & w() {
			return m_w;
		}
		
		inline T & x() {
			return m_x;
		}
		
		inline T & y() {
			return m_y;
		}
		
		inline T & z() {
			return m_z;
		}
		
		//inline T & operator[](size_t i) {
		//	assert(i < 4);
		//	return *(&m_w + i);
		//}
		
		inline quat<T> add(const quat<T> & rhs) const {
			return quat<T>(m_w + rhs.m_w, m_x + rhs.m_x, m_y + rhs.m_y, m_z + rhs.m_z);
		}
		
		inline quat<T> subtract(const quat<T> & rhs) const {
			return quat<T>(m_w - rhs.m_w, m_x - rhs.m_x, m_y - rhs.m_y, m_z - rhs.m_z);
		}
		
		inline quat<T> negate() const {
			return quat<T>(-m_w, -m_x, -m_y, -m_z);
		}
		
		inline quat<T> conjugate() const {
			return quat<T>(m_w, -m_x, -m_y, -m_z);
		}
		
		inline T norm() const {
			return math::sqrt(m_w * m_w + m_x * m_x + m_y * m_y + m_z * m_z);
		}
		
		inline quat<T> multiply(const T & f) const {
			return quat<T>(m_w * f, m_x * f, m_y * f, m_z * f);
		}
		
		inline quat<T> multiply(const quat<T> & rhs) const {
			return quat<T>(
				m_w * rhs.m_w - m_x * rhs.m_x - m_y * rhs.m_y - m_z * rhs.m_z,
				m_w * rhs.m_x + m_x * rhs.m_w + m_y * rhs.m_z - m_z * rhs.m_y,
				m_w * rhs.m_y - m_x * rhs.m_z + m_y * rhs.m_w + m_z * rhs.m_x,
				m_w * rhs.m_z + m_x * rhs.m_y - m_y * rhs.m_x + m_z * rhs.m_w
				);
		}
		
		inline quat<T> inverse() const {
			T inorm2 = 1 / (m_w * m_w + m_x * m_x + m_y * m_y + m_z * m_z);
			return quat<T>(inorm2 * m_w, -inorm2 * m_x, -inorm2 * m_y, -inorm2 * m_z);
		}
		
		inline quat<T> unit() const {
			return multiply(1 / norm());
		}
		
		inline T distance(const quat<T> &q) const {
			T dw = m_w - q.m_w;
			T dx = m_x - q.m_x;
			T dy = m_y - q.m_y;
			T dz = m_z - q.m_z;
			return math::sqrt(dw * dw + dx * dx + dy * dy + dz * dz);
		}
		
		inline vec3<T> rotate(const vec3<T> & v) const {
			// this * (0,v) * this^(-1)
			quat<T> q = this->multiply(quat<T>(0, v)).multiply(this->inverse());
			return vec3<T>(q.m_x, q.m_y, q.m_z);
		}
		
		inline T angle() const {
			return 2 * math::acos(m_w / norm());
		}
		
		inline vec3<T> axis() const {
			return ~vec3<T>(m_x, m_y, m_z);
		}
		
		inline quat<T> exp() const {
			T expw = math::exp(m_w);
			T vm = math::hypot(m_x, m_y, m_z);
			T ivm = 1 / vm;
			if (math::isinf(ivm)) {
				return quat<T>(expw * math::cos(vm), 0, 0, 0);
			} else {
				T vf = expw * math::sin(vm) * ivm;
				return quat<T>(expw * math::cos(vm), m_x * vf, m_y * vf, m_z * vf);
			}
		}
		
		inline quat<T> log() const {
			T qm = norm();
			T ivm = 1 / math::hypot(m_x, m_y, m_z);
			if (math::isinf(ivm)) {
				return quat<T>(math::log(qm), 0, 0, 0);
			} else {
				T vf = math::acos(m_w / qm) * ivm;
				return quat<T>(math::log(qm), m_x * vf, m_y * vf, m_z * vf);
			}
		}
		
		inline quat<T> pow(const T & power) const {
			T qm = norm();
			T theta = math::acos(m_w / qm);
			T ivm = 1 / math::hypot(m_x, m_y, m_z);
			if (math::isinf(ivm)) {
				return quat<T>().multiply(math::pow(qm, power));
			} else {
				T ivm_power_theta = ivm * power * theta;
				quat<T> p(0, m_x * ivm_power_theta, m_y * ivm_power_theta, m_z * ivm_power_theta);
				return p.exp().multiply(math::pow(qm, power));
			}
		}
		
		// vector component
		inline vec3<T> xyz() const {
			return vec3<T>(m_x, m_y, m_z);
		}
		
		// norm
		inline T operator+() const {
			return norm();
		}
		
		// unit
		inline quat<T> operator~() const {
			return unit();
		}
		
		// add
		inline quat<T> operator+(const quat<T> & rhs) const {
			return add(rhs);
		}
		
		// add-assign
		inline quat<T> & operator+=(const quat<T> & rhs) {
			return *this = add(rhs);
		}
		
		// subtract
		inline quat<T> operator-(const quat<T> & rhs) const {
			return subtract(rhs);
		}
		
		// subtract-assign
		inline quat<T> & operator-=(const quat<T> & rhs) {
			return *this = subtract(rhs);
		}
		
		// negate
		inline quat<T> operator-() const {
			return negate();
		}
		
		// multiply
		inline quat<T> operator*(const quat<T> & rhs) const {
			return multiply(rhs);
		}
		
		// multiply-assign
		inline quat<T> & operator*=(const quat<T> & rhs) {
			return *this = multiply(rhs);
		}
		
		// vector rotation
		inline vec3<T> operator*(const vec3<T> & rhs) const {
			return rotate(rhs);
		}
		
		// scalar multiply
		inline quat<T> operator*(const T & rhs) const {
			return multiply(rhs);
		}
		
		// scalar multiply-assign
		inline quat<T> & operator*=(const T & rhs) {
			return *this = multiply(rhs);
		}
		
		// scalar multiply, other order
		inline friend quat<T> operator*(const T & lhs, const quat<T> & rhs) {
			return rhs.multiply(lhs);
		}
		
		// scalar divide
		inline quat<T> operator/(const T & rhs) const {
			return multiply(1 / rhs);
		}
		
		// scalar divide-assign
		inline quat<T> & operator/=(const T & rhs) {
			return *this = multiply(1 / rhs);
		}
		
		// inverse
		inline quat<T> operator!() const {
			return inverse();
		}
		
		// cross-component-type implicit rotation vector cast
		template <typename U>
		inline operator vec3<U>() const {
			return angle() * axis();
		}
		
		// stream insertion
		inline friend std::ostream & operator<<(std::ostream & out, const quat<T> & q) {
			return out << '(' << q.w() << ", " << q.x() << ", " << q.y() << ", " << q.z() << ')';
		}
		
		inline static quat<T> zero() {
			return quat<T>(0, 0, 0, 0);
		}
		
		inline static quat<T> one() {
			return quat<T>();
		}
		
	};
	
	//
	// 4 x 4 matrix
	//
	template <typename T> 
	class mat4 {
		template <typename> friend class mat4;
		
	private:
		// row major
		T m_data[16];
		
		inline void clear() {
			for (T *pt = m_data + 16; pt --> m_data; ) {
				*pt = 0;
			}
		}
		
		inline static T det3x3(const T & e00, const T & e01, const T & e02,
						const T & e10, const T & e11, const T & e12,
						const T & e20, const T & e21, const T & e22) {
			T d = 0;
			d += e00 * e11 * e22;
			d += e01 * e12 * e20;
			d += e02 * e10 * e21;
			d -= e00 * e12 * e21;
			d -= e01 * e10 * e22;
			d -= e02 * e11 * e20;
			return d;
		}
		
	public:
		typedef T element_t;
		
		inline T operator()(size_t row, size_t col) const {
			assert(row < 4 && col < 4);
			return *(m_data + row * 4 + col);
		}
		
		// implicit pointer conversion; data is ROW MAJOR
		inline operator T const *() const {
			return m_data;
		}

		// using default copy constructor and assignment
		
		// default constructor, initialises to identity
		inline mat4() {
			clear();
			m_data[0] = 1;
			m_data[5] = 1;
			m_data[10] = 1;
			m_data[15] = 1;
		}
		
		// init as a multiple of the identity
		inline explicit mat4(const T & t) {
			clear();
			m_data[0] = t;
			m_data[5] = t;
			m_data[10] = t;
			m_data[15] = t;
		}
		
		// cross-component-type copy constructor
		template <typename U>
		inline mat4(const mat4<U> & other_) {
			T *pt = m_data + 16;
			U const *pu = other_.m_data + 16;
			while (pu--, pt --> m_data) {
				*pt = static_cast<T>(*pu);
			}
		}
		
		inline T & operator()(size_t row, size_t col) {
			assert(row < 4 && col < 4);
			return *(m_data + row * 4 + col);
		}
		
		inline mat4<T> add(const mat4<T> & rhs) const {
			mat4<T> m;
			T const *lpt = m_data + 16;
			T const *rpt = rhs.m_data + 16;
			T *mpt = m.m_data + 16;
			while (lpt--, rpt--, mpt --> m.m_data) {
				*mpt = *lpt + *rpt;
			}
			return m;
		}
		
		inline mat4<T> subtract(const mat4<T> & rhs) const {
			mat4<T> m;
			T const *lpt = m_data + 16;
			T const *rpt = rhs.m_data + 16;
			T *mpt = m.m_data + 16;
			while (lpt--, rpt--, mpt --> m.m_data) {
				*mpt = *lpt - *rpt;
			}
			return m;
		}
		
		inline mat4<T> multiply(const T & f) const {
			mat4<T> m;
			T const *pt = m_data + 16;
			T *mpt = m.m_data + 16;
			while (pt--, mpt --> m.m_data) {
				*mpt = *pt * f;
			}
			return m;
		}
		
		inline mat4<T> multiply(const mat4<T> & rhs) const {
			mat4<T> m;
			T const *lpt = m_data;
			T const *rpt = rhs.m_data;
			T *mpt = m.m_data;
			// unrolling ftw!
			mpt[0] = lpt[0] * rpt[0] + lpt[1] * rpt[4] + lpt[2] * rpt[8] + lpt[3] * rpt[12];
			mpt[1] = lpt[0] * rpt[1] + lpt[1] * rpt[5] + lpt[2] * rpt[9] + lpt[3] * rpt[13];
			mpt[2] = lpt[0] * rpt[2] + lpt[1] * rpt[6] + lpt[2] * rpt[10] + lpt[3] * rpt[14];
			mpt[3] = lpt[0] * rpt[3] + lpt[1] * rpt[7] + lpt[2] * rpt[11] + lpt[3] * rpt[15];
			mpt[4] = lpt[4] * rpt[0] + lpt[5] * rpt[4] + lpt[6] * rpt[8] + lpt[7] * rpt[12];
			mpt[5] = lpt[4] * rpt[1] + lpt[5] * rpt[5] + lpt[6] * rpt[9] + lpt[7] * rpt[13];
			mpt[6] = lpt[4] * rpt[2] + lpt[5] * rpt[6] + lpt[6] * rpt[10] + lpt[7] * rpt[14];
			mpt[7] = lpt[4] * rpt[3] + lpt[5] * rpt[7] + lpt[6] * rpt[11] + lpt[7] * rpt[15];
			mpt[8] = lpt[8] * rpt[0] + lpt[9] * rpt[4] + lpt[10] * rpt[8] + lpt[11] * rpt[12];
			mpt[9] = lpt[8] * rpt[1] + lpt[9] * rpt[5] + lpt[10] * rpt[9] + lpt[11] * rpt[13];
			mpt[10] = lpt[8] * rpt[2] + lpt[9] * rpt[6] + lpt[10] * rpt[10] + lpt[11] * rpt[14];
			mpt[11] = lpt[8] * rpt[3] + lpt[9] * rpt[7] + lpt[10] * rpt[11] + lpt[11] * rpt[15];
			mpt[12] = lpt[12] * rpt[0] + lpt[13] * rpt[4] + lpt[14] * rpt[8] + lpt[15] * rpt[12];
			mpt[13] = lpt[12] * rpt[1] + lpt[13] * rpt[5] + lpt[14] * rpt[9] + lpt[15] * rpt[13];
			mpt[14] = lpt[12] * rpt[2] + lpt[13] * rpt[6] + lpt[14] * rpt[10] + lpt[15] * rpt[14];
			mpt[15] = lpt[12] * rpt[3] + lpt[13] * rpt[7] + lpt[14] * rpt[11] + lpt[15] * rpt[15];
			return m;
		}
		
		inline vec4<T> multiply(const vec4<T> & rhs) const {
			T const *pt = m_data;
			return vec4<T>(pt[0] * rhs.x() + pt[1] * rhs.y() + pt[2] * rhs.z() + pt[3] * rhs.w(),
							pt[4] * rhs.x() + pt[5] * rhs.y() + pt[6] * rhs.z() + pt[7] * rhs.w(),
							pt[8] * rhs.x() + pt[9] * rhs.y() + pt[10] * rhs.z() + pt[11] * rhs.w(),
							pt[12] * rhs.x() + pt[13] * rhs.y() + pt[14] * rhs.z() + pt[15] * rhs.w());
		}
		
		inline mat4<T> negate() const {
			mat4<T> m;
			T const *pt = m_data + 16;
			T *mpt = m.m_data + 16;
			while (pt--, mpt --> m.m_data) {
				*mpt = -(*pt);
			}
			return m;
		}
		
		inline mat4<T> transpose() const {
			mat4<T> m;
			T const *pt = m_data;
			T *mpt = m.m_data;
			mpt[0] = pt[0];
			mpt[4] = pt[1];
			mpt[1] = pt[4];
			mpt[8] = pt[2];
			mpt[2] = pt[8];
			mpt[12] = pt[3];
			mpt[3] = pt[12];
			mpt[5] = pt[5];
			mpt[9] = pt[6];
			mpt[6] = pt[9];
			mpt[13] = pt[7];
			mpt[7] = pt[13];
			mpt[10] = pt[10];
			mpt[14] = pt[11];
			mpt[11] = pt[14];
			mpt[15] = pt[15];
			return m;
		}
		
		inline T determinant() const {
			T const *pt = m_data;
			T d = 0;
			// expand about first row
			d += pt[0] * det3x3(pt[5], pt[6], pt[7], pt[9], pt[10], pt[11], pt[13], pt[14], pt[15]);
			d -= pt[1] * det3x3(pt[4], pt[6], pt[7], pt[8], pt[10], pt[11], pt[12], pt[14], pt[15]);
			d += pt[2] * det3x3(pt[4], pt[5], pt[7], pt[8], pt[9], pt[11], pt[12], pt[13], pt[15]);
			d -= pt[3] * det3x3(pt[4], pt[5], pt[6], pt[8], pt[9], pt[10], pt[12], pt[13], pt[14]);
			return d;
		}
		
		inline mat4<T> inverse() const {
			mat4<T> m;
			T const *pt = m_data;
			T *mpt = m.m_data;
			// first row of cofactors, can use for determinant
			T c00 = det3x3(pt[5], pt[6], pt[7], pt[9], pt[10], pt[11], pt[13], pt[14], pt[15]);
			T c01 = -det3x3(pt[4], pt[6], pt[7], pt[8], pt[10], pt[11], pt[12], pt[14], pt[15]);
			T c02 = det3x3(pt[4], pt[5], pt[7], pt[8], pt[9], pt[11], pt[12], pt[13], pt[15]);
			T c03 = -det3x3(pt[4], pt[5], pt[6], pt[8], pt[9], pt[10], pt[12], pt[13], pt[14]);
			// get determinant by expanding about first row
			T invdet = 1 / (pt[0] * c00 + pt[1] * c01 + pt[2] * c02 + pt[3] * c03);
			// FIXME proper detect infinite determinant
			if (math::isinf(invdet) || invdet != invdet || invdet == 0)
				throw std::runtime_error("Non-invertible matrix.");
			// transpose of cofactor matrix * (1 / det)
			mpt[0] = c00 * invdet;
			mpt[4] = c01 * invdet;
			mpt[8] = c02 * invdet;
			mpt[12] = c03 * invdet;
			mpt[1] = -det3x3(pt[1], pt[2], pt[3], pt[9], pt[10], pt[11], pt[13], pt[14], pt[15]) * invdet;
			mpt[5] = det3x3(pt[0], pt[2], pt[3], pt[8], pt[10], pt[11], pt[12], pt[14], pt[15]) * invdet;
			mpt[9] = -det3x3(pt[0], pt[1], pt[3], pt[8], pt[9], pt[11], pt[12], pt[13], pt[15]) * invdet;
			mpt[13] = det3x3(pt[0], pt[1], pt[2], pt[8], pt[9], pt[10], pt[12], pt[13], pt[14]) * invdet;
			mpt[2] = det3x3(pt[1], pt[2], pt[3], pt[5], pt[6], pt[7], pt[13], pt[14], pt[15]) * invdet;
			mpt[6] = -det3x3(pt[0], pt[2], pt[3], pt[4], pt[6], pt[7], pt[12], pt[14], pt[15]) * invdet;
			mpt[10] = det3x3(pt[0], pt[1], pt[3], pt[4], pt[5], pt[7], pt[12], pt[13], pt[15]) * invdet;
			mpt[14] = -det3x3(pt[0], pt[1], pt[2], pt[4], pt[5], pt[6], pt[12], pt[13], pt[14]) * invdet;
			mpt[3] = -det3x3(pt[1], pt[2], pt[3], pt[5], pt[6], pt[7], pt[9], pt[10], pt[11]) * invdet;
			mpt[7] = det3x3(pt[0], pt[2], pt[3], pt[4], pt[6], pt[7], pt[8], pt[10], pt[11]) * invdet;
			mpt[11] = -det3x3(pt[0], pt[1], pt[3], pt[4], pt[5], pt[7], pt[8], pt[9], pt[11]) * invdet;
			mpt[15] = det3x3(pt[0], pt[1], pt[2], pt[4], pt[5], pt[6], pt[8], pt[9], pt[10]) * invdet;
			return m;
		}
		
		// add
		inline mat4<T> operator+(const mat4<T> & rhs) const {
			return add(rhs);
		}
		
		// add-assign
		inline mat4<T> & operator+=(const mat4<T> & rhs) {
			return *this = add(rhs);
		}
		
		// subtract
		inline mat4<T> operator-(const mat4<T> & rhs) const {
			return subtract(rhs);
		}
		
		// subtract-assign
		inline mat4<T> & operator-=(const mat4<T> & rhs) {
			return *this = subtract(rhs);
		}
		
		// negate
		inline mat4<T> operator-() const {
			return negate();
		}
		
		// multiply
		inline mat4<T> operator*(const mat4<T> & rhs) const {
			return multiply(rhs);
		}
		
		// multiply-assign
		inline mat4<T> & operator*=(const mat4<T> & rhs) {
			return *this = multiply(rhs);
		}
		
		// scalar multiply
		inline mat4<T> operator*(const T & rhs) const {
			return multiply(rhs);
		}
		
		// scalar multiply-assign
		inline mat4<T> & operator*=(const T & rhs) {
			return *this = multiply(rhs);
		}
		
		// scalar multiply, other order
		inline friend mat4<T> operator*(const T & lhs, const mat4<T> & rhs) {
			return rhs->multiply(lhs);
		}
		
		// scalar divide
		inline mat4<T> operator/(const T & rhs) const {
			return multiply(1 / rhs);
		}
		
		// scalar divide-assign
		inline mat4<T> operator/=(const T & rhs) {
			return *this = multiply(1 / rhs);
		}
		
		// determinant
		inline T operator+() const {
			return determinant();
		}
		
		// transpose
		inline mat4<T> operator~() const {
			return transpose();
		}
		
		// inverse
		inline mat4<T> operator!() const {
			return inverse();
		}
		
		// vector transform
		inline vec4<T> operator*(const vec4<T> & rhs) const {
			return multiply(rhs);
		}
		
		// stream insertion
		inline friend std::ostream & operator<<(std::ostream & out, const mat4<T> & m) {
			const size_t field_width = 10;
			std::ostringstream oss;
			oss << std::setprecision(4);
			oss << '[' << std::setw(field_width) << m(0, 0) << ", " << std::setw(field_width) << m(0, 1) << ", "
				<< std::setw(field_width) << m(0, 2) << ", " << std::setw(field_width) << m(0, 3) << ']' << std::endl;
			oss << '[' << std::setw(field_width) << m(1, 0) << ", " << std::setw(field_width) << m(1, 1) << ", "
				<< std::setw(field_width) << m(1, 2) << ", " << std::setw(field_width) << m(1, 3) << ']' << std::endl;
			oss << '[' << std::setw(field_width) << m(2, 0) << ", " << std::setw(field_width) << m(2, 1) << ", "
				<< std::setw(field_width) << m(2, 2) << ", " << std::setw(field_width) << m(2, 3) << ']' << std::endl;
			oss << '[' << std::setw(field_width) << m(3, 0) << ", " << std::setw(field_width) << m(3, 1) << ", "
				<< std::setw(field_width) << m(3, 2) << ", " << std::setw(field_width) << m(3, 3) << ']';
			return out << oss.str();
		}

		inline static mat4<T> shear(int t_dim, int s_dim, const T & f) {
			mat4<T> m;
			m(t_dim, s_dim) = f;
			return m;
		}
		
		inline static mat4<T> translate(const T & dx, const T & dy, const T & dz) {
			mat4<T> m;
			m(0, 3) = dx;
			m(1, 3) = dy;
			m(2, 3) = dz;
			return m;
		}
		
		inline static mat4<T> translate(const vec3<T> & d) {
			return mat4<T>::translate(d.x(), d.y(), d.z());
		}
		
		inline static mat4<T> scale(const T & fx, const T & fy, const T & fz) {
			mat4<T> m;
			m(0, 0) = fx;
			m(1, 1) = fy;
			m(2, 2) = fz;
			return m;
		}
		
		inline static mat4<T> scale(const vec3<T> & f) {
			return mat4<T>::scale(f.x(), f.y(), f.z());
		}
		
		inline static mat4<T> scale(const T & f) {
			return mat4<T>::scale(f, f, f);
		}
		
		inline static mat4<T> rotateX(const T & angle) {
			mat4<T> m;
			m(1, 1) = math::cos(angle);
			m(1, 2) = -math::sin(angle);
			m(2, 1) = math::sin(angle);
			m(2, 2) = math::cos(angle);
			return m;
		}
		
		inline static mat4<T> rotateY(const T & angle) {
			mat4<T> m;
			m(0, 0) = math::cos(angle);
			m(0, 2) = math::sin(angle);
			m(2, 0) = -math::sin(angle);
			m(2, 2) = math::cos(angle);
			return m;
		}
		
		inline static mat4<T> rotateZ(const T & angle) {
			mat4<T> m;
			m(0, 0) = math::cos(angle);
			m(0, 1) = -math::sin(angle);
			m(1, 0) = math::sin(angle);
			m(1, 1) = math::cos(angle);
			return m;
		}
		
		inline static mat4<T> rotate(const quat<T> & q) {
			mat4<T> m;
			T w = q.w(), x = q.x(), y = q.y(), z = q.z();
			m(0, 0) = w * w + x * x - y * y - z * z;
			m(1, 0) = 2 * x * y + 2 * w * z;
			m(2, 0) = 2 * x * z - 2 * w * y;
			m(0, 1) = 2 * x * y - 2 * w * z;
			m(1, 1) = w * w - x * x + y * y - z * z;
			m(2, 1) = 2 * y * z + 2 * w * x;
			m(0, 2) = 2 * x * z + 2 * w * y;
			m(1, 2) = 2 * y * z - 2 * w * x;
			m(2, 2) = w * w - x * x - y * y + z * z;
			m(3, 3) = w * w + x * x + y * y + z * z;
			return m;
		}
		
	};

	//
	// Color
	//
	class color {
	public:
		float r, g, b, a;

		// using default copy constructor and assignment

		// constructor (transparent black)
		inline color() : r(0), g(0), b(0), a(0) { }

		// element-wise constructor
		inline color(float r_, float g_, float b_, float a_ = 1) : r(r_), g(g_), b(b_), a(a_) { }

		inline float operator[](size_t i) const {
			assert(i < 4);
			return *(&r + i);
		}

		inline float & operator[](size_t i) {
			assert(i < 4);
			return *(&r + i);
		}

		// implicit pointer cast
		inline operator float const *() const {
			return &r;
		}

		// TODO stream insertion

		// TODO fromARGB etc

		// TODO clamp before conversion to RGB

		unsigned int toARGB() {
			assert(sizeof(int) >= 4);
			return (((unsigned int) (a * 255.0f)) << 24) | (((unsigned int) (r * 255.0f)) << 16) |
				(((unsigned int) (g * 255.0f)) << 8) | ((unsigned int) (b * 255.0f));
		}

		unsigned int toRGB() {
			assert(sizeof(int) >= 4);
			return 0xFF000000 | (((unsigned int) (r * 255.0f)) << 16) |
				(((unsigned int) (g * 255.0f)) << 8) | ((unsigned int) (b * 255.0f));
		}

		// TODO math (add, mul, etc), clamp, ...

		static color white() {
			return color(1, 1, 1);
		}

		static color black() {
			return color(0, 0, 0);
		}

		static color red() {
			return color(1, 0, 0);
		}

		static color green() {
			return color(0, 1, 0);
		}

		static color blue() {
			return color(0, 0, 1);
		}

		static color yellow() {
			return color(1, 1, 0);
		}

		static color cyan() {
			return color(0, 1, 1);
		}

		static color magenta() {
			return color(1, 0, 1);
		}

		static color orange() {
			return color(1, 0.5, 0);
		}

	};

}

#endif // INITIAL3D_H

