#ifndef EVENT_HPP
#define EVENT_HPP

#include <vector>

namespace ambition {
	template <class T>
	class Delegate {
	public:
		Delegate() {}
		virtual ~Delegate() {}
		virtual void fire(T*) = 0;
	};

	template <class T>
	class Event {
		vector<Delegate<T> *> observers;
	public:
		Event() {}
		virtual ~Event() {}
		void attach(Delegate<T> &d) { observers.push_back(&d); }
		void notify() {
			
			for(typename std::vector<Delegate<T> *>::iterator it=observers.begin(); it != observers.end(); it++) {
				(*it)->fire(static_cast<T*>(this));
			}
		}
	};
}

#endif