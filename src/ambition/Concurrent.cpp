
#include <vector>

#include "Concurrent.hpp"
#include "Log.hpp"

using namespace std;

namespace ambition {

	mutex InterruptManager::m_mutex;
	map<thread::id, InterruptManager::thread_data_t> InterruptManager::m_thread_data;

	// wait on a condition variable.
	// lock should already be locked.
	void InterruptManager::wait(condition_variable &cond, unique_lock<mutex> &lock) {
		// caller should have locked mutex
		assert(lock.owns_lock());
		auto id = this_thread::get_id();
		// setup interruption mechanism
		{
			lock_guard<mutex> lock1(m_mutex);
			auto it = m_thread_data.find(id);
			if (it != m_thread_data.end()) {
				// thread data already present
				thread_data_t td = it->second;
				// remove the thread data
				m_thread_data.erase(it);
				assert(td.condition == nullptr);
				assert(td.mutex == nullptr);
				if (td.interrupt) {
					// interrupt scheduled, dont bother to wait
					throw interruption();
				}
			}
			// register cond/mutex with thread
			thread_data_t td;
			td.condition = &cond;
			td.mutex = lock.mutex();
			td.interrupt = false;
			m_thread_data[id] = td;
		}
		// actually wait
		cond.wait(lock);
		// check for interrupt and teardown
		{
			lock_guard<mutex> lock1(m_mutex);
			auto it = m_thread_data.find(id);
			// no other thread should have removed the thread data
			assert(it != m_thread_data.end());
			thread_data_t td = it->second;
			// remove the thread data
			m_thread_data.erase(it);
			if (td.interrupt) {
				// thread was interrupted
				throw interruption();
			}
		}
	}

	// interrupt a thread waiting on a condition variable.
	// if thread is not waiting, it will be interrupted when next it does.
	void InterruptManager::interrupt(const std::thread::id &id) {
		thread_data_t td;
		{
			// first determine what condition var to notify, if any
			lock_guard<mutex> lock(m_mutex);
			auto it = m_thread_data.find(id);
			if (it == m_thread_data.end()) {
				// thread not waiting, schedule interrupt and return
				td.condition = nullptr;
				td.mutex = nullptr;
				td.interrupt = true;
				m_thread_data[id] = td;
				return;
			}
			// we found the cond/mutex of the thread to be interrupted, set interrupt status
			it->second.interrupt = true;
			td = it->second;
		}
		{
			// locking the mutex used for the condition wait ensures any other threads
			// are either completely outside the wait sequence or are actually waiting
			lock_guard<mutex> lock(*td.mutex);
		}
		// wake the interrupted thread - this may (will) cause spurious wakeup of other threads
		td.condition->notify_all();
	}

	// interrupt all threads waiting on a condition variable.
	// the mutex this condition variable is waiting with is assumed to be locked already.
	void InterruptManager::interrupt(condition_variable &cond) {
		vector<thread_data_t> tdv;
		{
			// find waiting threads
			lock_guard<mutex> lock(m_mutex);
			for (auto it = m_thread_data.begin(); it != m_thread_data.end(); it++) {
				if (it->second.condition == &cond) {
					// set interrupt
					it->second.interrupt = true;
					tdv.push_back(it->second);
				}
			}
		}
		// notify waiting threads
		cond.notify_all();
	}

	bool AsyncExecutor::m_started(false);
	blocking_queue<AsyncExecutor::task_t> AsyncExecutor::m_fast_queue;
	blocking_queue<AsyncExecutor::task_t> AsyncExecutor::m_slow_queue;
	thread AsyncExecutor::m_fast_thread;
	thread AsyncExecutor::m_slow_thread;
	thread::id AsyncExecutor::m_main_id;
	mutex AsyncExecutor::m_exec_mutex;
	map<thread::id, blocking_queue<AsyncExecutor::task_t>> AsyncExecutor::m_exec_queues;

}