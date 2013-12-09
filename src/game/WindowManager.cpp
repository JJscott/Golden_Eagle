#include <string>
#include <stdexcept>
#include <cstring>
#include <vector>

#include <GLFW/glfw3.h>

#include "WindowManager.hpp"
#include "Window.hpp"
#include "common/Log.hpp"



namespace ambition {
	class WindowManager::WindowManagerImpl {
		bool isGLFWinit = false;
		std::vector<Window*> windows;

	public:
		void init();
		Window* addWindow(int, int, const char*);
		void apply();
	};

	WindowManager::WindowManager() {
		manager_ = new WindowManagerImpl;
	}

	WindowManager::~WindowManager() {
		delete manager_;
	}

	void WindowManager::init() {
		manager_->init();
	}

	Window* WindowManager::addWindow(int w, int h, const char* title) {
		return manager_->addWindow(w, h, title);
	}

	void WindowManager::apply() {
		manager_->apply();
	}

	class windowManager_error : public std::runtime_error {
		public:
		explicit inline windowManager_error(const std::string & what_ = "Window manager error") : 
			std::runtime_error(what_) { }
	};

	void WindowManager::WindowManagerImpl::init() {
		if(!isGLFWinit) {
			// glfwSetErrorCallback(error_callback);
			if (!glfwInit()) {
				log("WindowManager") % Log::nope << "GLFW Initialisation failed";
				throw windowManager_error("GLFW Initalisation failed");
			}

		    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		    glfwWindowHint(GLFW_SAMPLES, 16);

			log("WindowManager") << "GLFW Initialised";
		}
	}

	Window* WindowManager::WindowManagerImpl::addWindow(int w, int h, const char* title) {
		Window *nW = new Window(w, h, title);
		windows.push_back(nW);
		return nW;
	}

	void WindowManager::WindowManagerImpl::apply() {
		for(Window *w: windows) {
			if(!w->isInit())
				w->init();
		}
	}
}
