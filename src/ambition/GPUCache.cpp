


#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <queue>
#include <vector>

#include "GPUCache.hpp"

using namespace std;

namespace ambition {

	size_t GPUCacheManager::m_max_memCount = 0;
	size_t GPUCacheManager::m_cur_memCount = 0;
	std::vector<GPUCacheable *> GPUCacheManager::m_cache;

	void GPUCacheManager::setMaxMemory(size_t mem) {
		GPUCacheManager::m_max_memCount = mem;
		GPUCacheManager::alloc(0);
	}

	size_t GPUCacheManager::getMaxMemory() {
		return m_max_memCount;
	}

	size_t GPUCacheManager::getCurrentMemory() {
		return m_cur_memCount;
	}

	void GPUCacheManager::alloc(size_t mem) {
		GPUCacheManager::m_cur_memCount += mem;

		if (GPUCacheManager::m_cur_memCount >= GPUCacheManager::m_max_memCount) {
			//create queue here

			struct elem {
				GPUCacheable *c;
				elem(GPUCacheable *c_) : c(c_) { }
				int operator<(const elem &rhs) const {
					return c->timestamp() > rhs.c->timestamp();
				}
			};

			priority_queue<elem> q;

			for (GPUCacheable *c : GPUCacheManager::m_cache) {
				if (c->usage() > 0) {
					q.push(elem(c));
				}
			}

			while (GPUCacheManager::m_cur_memCount >= GPUCacheManager::m_max_memCount && !q.empty()) {
				//remove from queue
				GPUCacheable *c = q.top().c;
				q.pop();
				c->unload();
			}
		}

		// cout << "Current Memory usage : " << m_cur_memCount << " / " <<  m_max_memCount << endl;
	}

	void GPUCacheManager::free(size_t mem) {
		assert(mem <= GPUCacheManager::m_cur_memCount);
		GPUCacheManager::m_cur_memCount -= mem;
	}

	void GPUCacheManager::add(GPUCacheable *c) {
		GPUCacheManager::m_cache.push_back(c);
	}

	void GPUCacheManager::remove(GPUCacheable *c) {
		auto it = std::find(GPUCacheManager::m_cache.begin(), GPUCacheManager::m_cache.end(), c);
		if (it != GPUCacheManager::m_cache.end()) {
			GPUCacheManager::m_cache.erase(it);
		}
	}

	GPUCacheManager::timestamp_t GPUCacheManager::now() { return std::chrono::steady_clock::now(); }
}
