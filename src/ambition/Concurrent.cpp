
#include <vector>

#include "Concurrent.hpp"
#include "Log.hpp"

using namespace std;

namespace ambition {

	mutex InterruptManager::m_mutex;
	map<thread::id, InterruptManager::thread_data_t> InterruptManager::m_thread_data;

	// wait on a condition variable
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
			// locking the mutex used for the condition wait ensures the thread
			// is _actually_ waiting before we wake it
			lock_guard<mutex> lock(*td.mutex);
		}
		// wake the interrupted thread - this may (will) cause spurious wakeup of other threads
		td.condition->notify_all();
	}

	// interrupt all threads waiting on a condition variable
	void InterruptManager::interrupt(std::condition_variable &cond) {
		vector<thread_data_t> tdv;
		{
			// find waiting threads
			lock_guard<mutex> lock(m_mutex);
			for (auto pair : m_thread_data) {
				if (pair.second.condition == &cond) {
					pair.second.interrupt = true;
					tdv.push_back(pair.second);
				}
			}
		}
		// interrupt them
		for (auto td : tdv) {
			{
				// locking the mutex used for the condition wait ensures the thread
				// is _actually_ waiting before we wake it
				lock_guard<mutex> lock(*td.mutex);
			}
			td.condition->notify_all();
		}
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
					} catch (...) {
						log("AsyncExec").error() << "Uncaught exception (not derived from std::exception)";
					}
				}
			});
			m_started = true;
		}
	}

	// stop the background thread. must be called before exit() to die nicely.
	void AsyncExecutor::stop() {
		if (m_started) {
			log("AsyncExec") % 0 << "Stopping background thread...";
			// give the last log message time to show up
			this_thread::sleep_for(chrono::milliseconds(10));
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