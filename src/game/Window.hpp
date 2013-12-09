#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <GLFW/glfw3.h>
#include "common/Log.hpp"
#include <cstdio>

namespace ambition {
	class Window {
		GLFWwindow* handle;
		int width;
		int height;
		char* title;

		bool isInit_ = false;
		public:
		Window(int w, int h, const char* t) { width = w; height = h; title = new char[256]; strcpy(title, t); }
		bool isInit() { return isInit_; }
		void init() { 
			handle = glfwCreateWindow(width, height, title, NULL, NULL);
			if (!handle) {
				log("GLFW") % Log::nope << "Window creation failed";
				glfwTerminate();
			}
			log("GLFW") << "Window created";

			glfwMakeContextCurrent(handle);
		}
		GLFWwindow* getHandle() {
			return handle;
		}

		void setTitle(const char* newTitle) {
			memset(title, '\0', 256);
			strcpy(title, newTitle);
			glfwSetWindowTitle(handle, title);
		}
	};
}

#endif
