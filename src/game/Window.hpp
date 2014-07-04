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

// this is to enable multiple context support
namespace ambition {
	void * glewGetContextImpl();
}
#define glewGetContext() ((GLEWContext *) ambition::glewGetContextImpl())
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <ambition/Log.hpp>
#include <ambition/Concurrent.hpp>

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

	template <typename T>
	inline point2<T> operator*(const point2<T> &lhs, const T &rhs) {
		point2<T> r;
		r.x = lhs.x * rhs;
		r.y = lhs.y * rhs;
		return r;
	}

	template <typename T>
	inline point2<T> operator/(const point2<T> &lhs, const T &rhs) {
		point2<T> r;
		r.x = lhs.x / rhs;
		r.y = lhs.y / rhs;
		return r;
	}

	template <typename T>
	inline size2<T> operator*(const size2<T> &lhs, const T &rhs) {
		size2<T> r;
		r.w = lhs.w * rhs;
		r.h = lhs.h * rhs;
		return r;
	}

	template <typename T>
	inline size2<T> operator/(const size2<T> &lhs, const T &rhs) {
		size2<T> r;
		r.w = lhs.w / rhs;
		r.h = lhs.h / rhs;
		return r;
	}

	template <typename T>
	inline point2<T> operator+(const point2<T> &lhs, const size2<T> &rhs) {
		point2<T> r;
		r.x = lhs.x + rhs.w;
		r.y = lhs.y + rhs.h;
		return r;
	}

	template <typename T>
	inline point2<T> operator-(const point2<T> &lhs, const size2<T> &rhs) {
		point2<T> r;
		r.x = lhs.x - rhs.w;
		r.y = lhs.y - rhs.h;
		return r;
	}

	template <typename T>
	inline size2<T> operator+(const size2<T> &lhs, const size2<T> &rhs) {
		size2<T> r;
		r.w = lhs.w + rhs.w;
		r.h = lhs.h + rhs.h;
		return r;
	}

	template <typename T>
	inline size2<T> operator-(const size2<T> &lhs, const size2<T> &rhs) {
		size2<T> r;
		r.w = lhs.w - rhs.w;
		r.h = lhs.h - rhs.h;
		return r;
	}

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

	// Thin wrapper around GLFW windowing.
	// Each window can only be used on one thread at once.
	class Window : private Uncopyable {
	private:
		GLFWwindow* m_handle;
		bool m_glew_init_done = false;

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

		inline Window(GLFWwindow *handle_) : m_handle(handle_) {
			if (m_handle == nullptr) throw window_error("GLFW window handle is null");
			initialise();
		}

		inline GLFWwindow * handle() const {
			return m_handle;
		}

		inline void pos(int x, int y) {
			glfwSetWindowPos(m_handle, x, y);
		}

		inline void pos(const point2i &p) {
			glfwSetWindowPos(m_handle, p.x, p.y);
		}

		inline point2i pos() const {
			point2i p;
			glfwGetWindowPos(m_handle, &p.x, &p.y);
			return p;
		}

		inline void size(int w, int h) {
			glfwSetWindowSize(m_handle, w, h);
		}

		inline void size(const size2i &s) {
			glfwSetWindowSize(m_handle, s.w, s.h);
		}

		inline size2i size() const {
			size2i s;
			glfwGetWindowSize(m_handle, &s.w, &s.h);
			return s;
		}

		inline void width(int w) {
			size2i s = size();
			s.w = w;
			size(s);
		}

		inline int width() const {
			int w, h;
			glfwGetWindowSize(m_handle, &w, &h);
			return w;
		}

		inline void height(int h) {
			size2i s = size();
			s.h = h;
			size(s);
		}

		inline int height() const {
			int w, h;
			glfwGetWindowSize(m_handle, &w, &h);
			return h;
		}

		inline void title(const std::string &s) {
			glfwSetWindowTitle(m_handle, s.c_str());
		}

		inline void visible(bool b) {
			if (b) {
				glfwShowWindow(m_handle);
			} else {
				glfwHideWindow(m_handle);
			}
		}

		inline bool shouldClose() const {
			return glfwWindowShouldClose(m_handle);
		}

		void makeContextCurrent();

		inline void swapBuffers() const {
			glfwSwapBuffers(m_handle);
		}

		inline int attrib(int a) const {
			return glfwGetWindowAttrib(m_handle, a);
		}

		// get current state of a key
		bool getKey(int key);

		// get current state of a key, then clear it
		bool pollKey(int key);

		// get the current state of a mouse button
		bool getMouseButton(int button);

		// get the current state of a mouse button, then clear it
		bool pollMouseButton(int button);

		inline ~Window() {
			destroy();
		}
	};

	class create_window_args {
	private:
		size2i m_size = size2i(512, 512);
		std::string m_title = "";
		GLFWmonitor *m_monitor = nullptr;
		const Window *m_share = nullptr;
		std::map<int, int> m_hints;

	public:
		create_window_args() {
			m_hints[GLFW_OPENGL_PROFILE] = GLFW_OPENGL_CORE_PROFILE;
			m_hints[GLFW_CONTEXT_VERSION_MAJOR] = 3;
			m_hints[GLFW_CONTEXT_VERSION_MINOR] = 3;
			m_hints[GLFW_OPENGL_FORWARD_COMPAT] = true;
			m_hints[GLFW_SAMPLES] = 0;
			m_hints[GLFW_VISIBLE] = false;
		}

		inline create_window_args & width(int w) { m_size.w = w; return *this; }
		inline create_window_args & height(int h) { m_size.h = h; return *this; }
		inline create_window_args & size(int w, int h) { m_size.w = w; m_size.h = h; return *this; }
		inline create_window_args & size(size2i s) { m_size = s; return *this; }
		inline create_window_args & title(const std::string &title) { m_title = title; return *this; }
		inline create_window_args & monitor(GLFWmonitor *mon) { m_monitor = mon; return *this; }
		inline create_window_args & visible(bool b) { m_hints[GLFW_VISIBLE] = b; return *this; }
		inline create_window_args & resizable(bool b) { m_hints[GLFW_RESIZABLE] = b; return *this; }
		inline create_window_args & debug(bool b) { m_hints[GLFW_OPENGL_DEBUG_CONTEXT] = b; return *this; }
		inline create_window_args & share(const Window *win) { m_share = win; return *this; }
		inline create_window_args & hint(int target, int hint) { m_hints[target] = hint; return *this; }

		// this should only be called from the main thread
		operator Window * ();

	};

	inline create_window_args createWindow() {
		return create_window_args();
	}
}

#endif
