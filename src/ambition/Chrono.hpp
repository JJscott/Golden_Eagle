/*
 * Ambition Chrono Header
 *
 * Exists to fix MSVC's terribad clock implementations.
 * https://connect.microsoft.com/VisualStudio/feedback/details/719443/c-chrono-headers-high-resolution-clock-does-not-have-high-resolution
 *
 * Use the clock defined here instead of high_resolution_clock.
 */

#ifndef AMBITION_CHRONO_HPP
#define AMBITION_CHRONO_HPP

#include <chrono>

namespace ambition {

#if defined(_MSC_VER) && _MSC_VER <= 1800
// we're on MSVC prior to fixed stdlib, proceded with fix
#define AMBITION_CHRONO_MSVC_FIX

	class really_high_resolution_clock {
		// this is implemented using QueryPerformanceCounter
	public:
		using rep = std::chrono::nanoseconds::rep;
		using period = std::chrono::nanoseconds::period;
		using duration = std::chrono::nanoseconds;
		using time_point = std::chrono::time_point<really_high_resolution_clock>;

		static const bool is_steady = true;

		static time_point now();
	};

#else
	// assume compiler's std libraries implement it properly
	using really_high_resolution_clock = std::chrono::high_resolution_clock;

#endif // _MSC_VER

}

#endif // AMBITION_CHRONO_HPP