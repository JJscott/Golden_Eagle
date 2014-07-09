

#pragma once

#include <chrono>
#include <vector>

namespace ambition {

	class GPUCacheable;

	class GPUCacheManager {
	public:
		using timestamp_t = std::chrono::steady_clock::time_point;
		static void setMaxMemory(size_t);
		static size_t getMaxMemory();
		static size_t getCurrentMemory();
		static void alloc(size_t);
		static void free(size_t);
		static void add(GPUCacheable *);
		static void remove(GPUCacheable *);
		static timestamp_t now();
	private:
		static size_t m_max_memCount;
		static size_t m_cur_memCount;
		static std::vector<GPUCacheable *> m_cache;
	};

	class GPUCacheable {
	public:
		virtual size_t usage() = 0;
		virtual void unload() = 0;
		virtual GPUCacheManager::timestamp_t timestamp() = 0;
		virtual inline ~GPUCacheable() { }
	};

}