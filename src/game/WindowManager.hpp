//#ifndef WINDOWMANAGER_HPP
//#define WINDOWMANAGER_HPP 
//
//#include "Window.hpp"
//
//namespace ambition {
//	class WindowManager {
//		
//		class WindowManagerImpl;
//		WindowManagerImpl *manager_;
//
//		WindowManager(WindowManager const&);
//		void operator=(WindowManager const&);
//		
//
//		WindowManager();
//		~WindowManager();
//	public:
//		static WindowManager* getInstance() {
//			static WindowManager instance;
//			return &instance;
//		}
//		
//		// void setWindowSize(int w, int h) { manager_.setWindowSize(w, h); };
//		// void useMultipleMonitors(bool);
//
//		void init();
//		Window* addWindow(int, int, const char*);
//		void apply();
//	};
//
//	inline static WindowManager* wm() {
//		return WindowManager::getInstance();
//	}
//}
//
//#endif