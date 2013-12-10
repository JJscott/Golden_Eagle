
#include <map>
#include <bitset>

#include "Window.hpp"

namespace ambition {

	namespace {
		struct WindowData {
			Window *window;
			std::bitset<GLFW_KEY_LAST + 1> vk;
			std::bitset<GLFW_MOUSE_BUTTON_LAST + 1> mb;
			WindowData(Window *window_) : window(window_) { }
		};

		WindowData * getWindowData(GLFWwindow *handle) {
			return (WindowData *) glfwGetWindowUserPointer(handle);
		}

		Window * getWindow(GLFWwindow *handle) {
			return getWindowData(handle)->window;
		}

		void callbackWindowPos(GLFWwindow *handle, int x, int y) {
			Window *win = getWindow(handle);
			window_pos_event e;
			e.window = win;
			e.pos = point2i(x, y);
			win->onMove.notify(e);
		}

		void callbackWindowSize(GLFWwindow *handle, int w, int h) {
			Window *win = getWindow(handle);
			window_size_event e;
			e.window = win;
			e.size = size2i(w, h);
			win->onResize.notify(e);
		}

		void callbackWindowClose(GLFWwindow *handle) {
			Window *win = getWindow(handle);
			window_event e;
			e.window = win;
			win->onClose.notify(e);
		}

		void callbackWindowRefresh(GLFWwindow *handle) {
			Window *win = getWindow(handle);
			window_event e;
			e.window = win;
			win->onRefresh.notify(e);
		}

		void callbackWindowFocus(GLFWwindow *handle, int focused) {
			WindowData *wd = getWindowData(handle);
			if (!focused) {
				// lost focus, release all keys and buttons
				wd->vk.reset();
				wd->mb.reset();
			}
			window_focus_event e;
			e.window = wd->window;
			e.focused = focused;
			wd->window->onFocus.notify(e);
			if (focused) {
				wd->window->onFocusGain.notify(e);
			} else {
				wd->window->onFocusLose.notify(e);
			}
		}

		void callbackWindowIconify(GLFWwindow *handle, int iconified) {
			Window *win = getWindow(handle);
			window_icon_event e;
			e.window = win;
			e.iconified = iconified;
			win->onIcon.notify(e);
			if (iconified) {
				win->onMinimise.notify(e);
			} else {
				win->onRestore.notify(e);
			}
		}

		void callbackFramebufferSize(GLFWwindow *handle, int w, int h) {
			// TODO
		}

		void callbackMouseButton(GLFWwindow *handle, int button, int action, int mods) {
			// i dont think mouse buttons get repeats, but whatever
			WindowData *wd = getWindowData(handle);
			if (action == GLFW_PRESS || action == GLFW_REPEAT) {
				wd->vk.set(button, true);
			} else {
				wd->vk.set(button, false);
			}
			mouse_button_event e;
			e.window = wd->window;
			e.button = button;
			e.action = action;
			e.mods = mods;
			e.entered = false;
			e.exited = false;
			glfwGetCursorPos(handle, &e.pos.x, &e.pos.y);
			wd->window->onMouse.notify(e);
			if (action == GLFW_PRESS || action == GLFW_REPEAT) {
				wd->window->onMousePress.notify(e);
			} else {
				wd->window->onMouseRelease.notify(e);
			}
		}

		void callbackCursorPos(GLFWwindow *handle, double x, double y) {
			Window *win = getWindow(handle);
			mouse_event e;
			e.window = win;
			e.pos = point2d(x, y);
			e.entered = false;
			e.exited = false;
			win->onMouseMove.notify(e);
		}

		void callbackCursorEnter(GLFWwindow *handle, int entered) {
			Window *win = getWindow(handle);
			mouse_event e;
			e.window = win;
			glfwGetCursorPos(handle, &e.pos.x, &e.pos.y);
			e.entered = entered;
			e.exited = !entered;
			win->onMouseMove.notify(e);
			if (entered) {
				win->onMouseEnter.notify(e);
			} else {
				win->onMouseExit.notify(e);
			}
		}

		void callbackScroll(GLFWwindow *handle, double xoffset, double yoffset) {
			Window *win = getWindow(handle);
			mouse_scroll_event e;
			e.window = win;
			glfwGetCursorPos(handle, &e.pos.x, &e.pos.y);
			e.entered = false;
			e.exited = false;
			e.offset = size2d(xoffset, yoffset);
			win->onScroll.notify(e);
		}

		void callbackKey(GLFWwindow *handle, int key, int scancode, int action, int mods) {
			WindowData *wd = getWindowData(handle);
			if (action == GLFW_PRESS || action == GLFW_REPEAT) {
				wd->vk.set(key, true);
			} else {
				wd->vk.set(key, false);
			}
			key_event e;
			e.window = wd->window;
			e.key = key;
			e.scancode = scancode;
			e.action = action;
			e.mods = mods;
			wd->window->onKey.notify(e);
			if (action == GLFW_PRESS || action == GLFW_REPEAT) {
				wd->window->onKeyPress.notify(e);
			} else {
				wd->window->onKeyRelease.notify(e);
			}
		}

		void callbackChar(GLFWwindow *handle, unsigned codepoint) {
			Window *win = getWindow(handle);
			char_event e;
			e.window = win;
			e.codepoint = codepoint;
			win->onChar.notify(e);
		}
	}

	void Window::initialise() {
		// set ALL the callbacks
		glfwSetWindowPosCallback(m_handle, callbackWindowPos);
		glfwSetWindowSizeCallback(m_handle, callbackWindowSize);
		glfwSetWindowCloseCallback(m_handle, callbackWindowClose);
		glfwSetWindowRefreshCallback(m_handle, callbackWindowRefresh);
		glfwSetWindowFocusCallback(m_handle, callbackWindowFocus);
		glfwSetWindowIconifyCallback(m_handle, callbackWindowIconify);
		glfwSetFramebufferSizeCallback(m_handle, callbackFramebufferSize);
		glfwSetMouseButtonCallback(m_handle, callbackMouseButton);
		glfwSetCursorPosCallback(m_handle, callbackCursorPos);
		glfwSetCursorEnterCallback(m_handle, callbackCursorEnter);
		glfwSetScrollCallback(m_handle, callbackScroll);
		glfwSetKeyCallback(m_handle, callbackKey);
		glfwSetCharCallback(m_handle, callbackChar);
		// create a windowdata object
		glfwSetWindowUserPointer(m_handle, new WindowData(this));
	}

	void Window::destroy() {
		delete getWindowData(m_handle);
		glfwDestroyWindow(m_handle);
	}

	bool Window::getKey(int key) {
		return getWindowData(m_handle)->vk.test(key);
	}

	bool Window::pollKey(int key) {
		WindowData *wd = getWindowData(m_handle);
		bool b = wd->vk.test(key);
		wd->vk.reset(key);
		return b;
	}

	bool Window::getMouseButton(int button) {
		return getWindowData(m_handle)->mb.test(button);
	}

	bool Window::pollMouseButton(int button) {
		WindowData *wd = getWindowData(m_handle);
		bool b = wd->mb.test(button);
		wd->mb.reset(button);
		return b;
	}

}