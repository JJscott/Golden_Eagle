/*
 *
 * Ambition Window Management Header
 *
 */

#ifndef AMBITION_WINDOW_HPP
#define AMBITION_WINDOW_HPP

#include <string>
#include <stdexcept>
#include <map>

#include <GLFW/glfw3.h>
#include "ambition/Ambition.hpp"
#include "ambition/Log.hpp"
#include "ambition/Concurrent.hpp"

namespace ambition {

	//
	// Point2
	//
	template <typename T>
	struct point2 {
		T x, y;
		point2(T x_, T y_) : x(x_), y(y_) { }
		point2() : x(0), y(0) { }
	};

	using point2i = point2<int>;
	using point2d = point2<double>;

	//
	// Size2
	//
	template <typename T>
	struct size2 {
		T w, h;
		size2(T w_, T h_) : w(w_), h(h_) { }
		size2() : w(0), h(0) { }
	};

	using size2i = size2<int>;
	using size2d = size2<double>;

	// forward declaration
	class Window;

	//
	// Window Events
	//
	struct window_event { Window *window; };
	struct window_pos_event : public window_event { point2i pos; };
	struct window_size_event : public window_event { size2i size; };
	struct window_focus_event : public window_event { bool focused; };
	struct window_icon_event : public window_event { bool iconified; };
	struct mouse_event : public window_event { point2d pos; bool entered; bool exited; };
	struct mouse_button_event : public mouse_event { int button; int action; int mods; };
	struct mouse_scroll_event : public mouse_event { size2d offset; };
	struct key_event : public window_event { int key; int scancode; int action; int mods; };
	struct char_event : public window_event { unsigned codepoint; };

	class window_error : public std::runtime_error {
	public:
		explicit window_error(const std::string &what_ = "Window Error") : runtime_error(what_) { }
	};

	class Window : private Uncopyable {
	private:
		GLFWwindow* m_handle;

		void initialise();
		void destroy();

	public:
		// events
		Event<window_pos_event> onMove;
		Event<window_size_event> onResize;
		Event<window_event> onClose;
		Event<window_event> onRefresh;
		Event<window_focus_event> onFocus;
		Event<window_focus_event> onFocusGain;
		Event<window_focus_event> onFocusLose;
		Event<window_icon_event> onIcon;
		Event<window_icon_event> onMinimise;
		Event<window_icon_event> onRestore;
		Event<mouse_button_event> onMouse;
		Event<mouse_button_event> onMousePress;
		Event<mouse_button_event> onMouseRelease;
		Event<mouse_event> onMouseMove;
		Event<mouse_event> onMouseEnter;
		Event<mouse_event> onMouseExit;
		Event<mouse_scroll_event> onScroll;
		Event<key_event> onKey;
		Event<key_event> onKeyPress;
		Event<key_event> onKeyRelease;
		Event<char_event> onChar;

		Window(GLFWwindow *handle_) : m_handle(handle_) {
			if (m_handle == nullptr) throw window_error("GLFW window handle is null");
			initialise();
		}

		GLFWwindow * getHandle() const {
			return m_handle;
		}

		void setSize(int w, int h) {
			glfwSetWindowSize(m_handle, w, h);
		}

		void setTitle(const std::string &s) {
			glfwSetWindowTitle(m_handle, s.c_str());
		}

		void setVisible(bool b) {
			if (b) {
				glfwShowWindow(m_handle);
			} else {
				glfwHideWindow(m_handle);
			}
		}

		int getWidth() const {
			int w, h;
			glfwGetWindowSize(m_handle, &w, &h);
			return w;
		}

		int getHeight() const {
			int w, h;
			glfwGetWindowSize(m_handle, &w, &h);
			return h;
		}

		bool shouldClose() const {
			return glfwWindowShouldClose(m_handle);
		}

		void makeContextCurrent() const {
			glfwMakeContextCurrent(m_handle);
		}

		void swapBuffers() const {
			glfwSwapBuffers(m_handle);
		}

		// get current state of a key
		bool getKey(int key);

		// get current state of a key, then clear it
		bool pollKey(int key);

		// get the current state of a mouse button
		bool getMouseButton(int button);

		// get the current state of a mouse button, then clear it
		bool pollMouseButton(int button);

		~Window() {
			destroy();
		}
	};


	class create_window_args {
	private:
		int m_width = 640;
		int m_height = 480;
		std::string m_title = "";
		GLFWmonitor *m_monitor = nullptr;
		std::map<int, int> m_hints;

	public:
		create_window_args() {
			m_hints[GLFW_OPENGL_PROFILE] = GLFW_OPENGL_CORE_PROFILE;
			m_hints[GLFW_CONTEXT_VERSION_MAJOR] = 3;
			m_hints[GLFW_CONTEXT_VERSION_MINOR] = 3;
			// multisampling becomes complicated with some of the stuff i have in mind -Ben
			m_hints[GLFW_SAMPLES] = 4;
		}

		inline create_window_args & setWidth(int w) { m_width = w; return *this; }
		inline create_window_args & setHeight(int h) { m_height = h; return *this; }
		inline create_window_args & setSize(int w, int h) { m_width = w; m_height = h; return *this; }
		inline create_window_args & setTitle(const std::string &title) { m_title = title; return *this; }
		inline create_window_args & setMonitor(GLFWmonitor *mon) { m_monitor = mon; return *this; }
		inline create_window_args & setHint(int target, int hint) { m_hints[target] = hint; return *this; }

		inline Window * open() {
			if (!glfwInit()) {
				log("GLFW").error() << "Initialisation failed";
				throw window_error("GLFW initialisation failed");
			}
			glfwDefaultWindowHints();
			for (auto me : m_hints) {
				glfwWindowHint(me.first, me.second);
			}
			GLFWwindow *handle = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);
			glfwDefaultWindowHints();
			if (!handle) {
				log("GLFW").error() << "Window creation failed, title=" << m_title;
				throw window_error("Window creation failed");
			}
			glfwMakeContextCurrent(handle);
			log("GLFW") << "Window created";
			return new Window(handle);
		}

	};

	inline create_window_args createWindow() {
		return create_window_args();
	}
}

#endif
