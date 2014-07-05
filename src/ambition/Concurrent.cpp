
#include "Concurrent.hpp"
#include "Log.hpp"

using namespace std;

namespace ambition {

	std::mutex InterruptManager::m_mutex;
	std::map<std::thread::id, InterruptManager::thread_data_t> InterruptManager::m_thread_data;

	// wait on a condition variable
	void InterruptManager::wait(std::condition_variable &cond, std::unique_lock<std::mutex> &lock) {
		// caller should have locked mutex
		assert(lock.owns_lock());
		auto id = std::this_thread::get_id();
		// setup interruption mechanism
		{
			std::lock_guard<std::mutex> lock1(m_mutex);
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
			std::lock_guard<std::mutex> lock1(m_mutex);
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
			// lock this mutex seperately first, because we need to
			// lock the cond's mutex first when we lock both to match with wait()
			std::lock_guard<std::mutex> lock(m_mutex);
			auto it = m_thread_data.find(id);
			if (it == m_thread_data.end()) {
				// thread not waiting, schedule interrupt
				td.condition = nullptr;
				td.mutex = nullptr;
				td.interrupt = true;
				m_thread_data[id] = td;
				return;
			}
			// we found the cond/mutex of the thread to be interrupted
			td = it->second;
		}
		{
			// lock in the same order as wait()
			// locking the mutex used for the condition wait ensures the thread
			// is _actually_ waiting before we wake it
			std::lock_guard<std::mutex> lock(*td.mutex);
			std::lock_guard<std::mutex> lock1(m_mutex);
			// check we're still ok after re-locking
			auto it = m_thread_data.find(id);
			if (it == m_thread_data.end()) {
				// another thread did the interrupt?
				return;
			}
			// signal interruption
			it->second.interrupt = true;
		}
		// wake the interrupted thread - this may (will) cause spurious wakeup of other threads
		td.condition->notify_all();
	}

	atomic<bool> AsyncExecutor::m_started(false);
	blocking_queue<AsyncExecutor::task_t> AsyncExecutor::m_backQueue;
	blocking_queue<AsyncExecutor::task_t> AsyncExecutor::m_mainQueue;
	thread AsyncExecutor::m_thread;

	// start the background thread
	void AsyncExecutor::start() {
		if (!m_started) {
			log("AsyncExec") % 0 << "Starting...";
			m_thread = thread([] {
				log("AsyncExec") % 0 << "Background thread started";
				while (true) {
					task_t task;
					try {
						task = m_backQueue.pop();
					} catch (interruption &e) {
						// thread needs to quit
						break;
					}
					try {
						task();
					} catch (std::exception e) {
						log("AsyncExec").error() << "Uncaught exception; what(): " << e.what();
					}
				}
			});
			m_started = true;
		}
	}

	// stop the background thread. must be called before exit() to die nicely.
	void AsyncExecutor::stop() {
		if (m_started) {
			// you don't see this message most of the time because log uses AsyncExecutor
			log("AsyncExec") % 0 << "Stopping...";
			InterruptManager::interrupt(m_thread.get_id());
			// because of this
			// https://connect.microsoft.com/VisualStudio/feedback/details/747145/std-thread-join-hangs-if-called-after-main-exits-when-using-vs2012-rc
			// we can't register stop with atexit
			m_thread.join();
		}
	}

	// add a background task
	void AsyncExecutor::enqueueBackground(const task_t &f) {
		m_backQueue.push(f);
	}

	// add a task to the 'main' thread
	void AsyncExecutor::enqueueMain(const task_t &f) {
		m_mainQueue.push(f);
	}

	// execute tasks on the 'main' thread up to some time limit
	void AsyncExecutor::executeMain(double dt) {
		// TODO duration control
		while (!m_mainQueue.empty()) {
			m_mainQueue.pop()();
		}
	}
}