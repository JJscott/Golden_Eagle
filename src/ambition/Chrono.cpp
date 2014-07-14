
#include "Chrono.hpp"

#ifdef AMBITION_CHRONO_MSVC_FIX

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstdint>

namespace {

	inline int64_t queryPF() {
		static_assert(sizeof(LONGLONG) == sizeof(int64_t), "oops");
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		return freq.QuadPart;
	}

	inline int64_t getPF() {
		static int64_t freq = queryPF();
		return freq;
	}

}

namespace ambition {

	really_high_resolution_clock::time_point really_high_resolution_clock::now() {
		LARGE_INTEGER count;
		QueryPerformanceCounter(&count);
		int64_t s = count.QuadPart / getPF();
		int64_t r = count.QuadPart % getPF();
		int64_t ns = s * 1000000000LL + r * 1000000000LL / getPF();
		return time_point(std::chrono::nanoseconds(ns));
	}

}

#endif