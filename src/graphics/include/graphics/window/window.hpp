#ifndef GRAPHICS_WINDOW_WINDOW_HPP
#define GRAPHICS_WINDOW_WINDOW_HPP

struct GLFWwindow;

namespace graphics::window {

	class Window final {
	public:
		Window();
		Window(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(const Window&) = delete;
		Window& operator=(Window&&) = delete;
		~Window();

		// Create a new window
		bool create();

	private:
		GLFWwindow* handle;
	};

}

#endif // !GRAPHICS_WINDOW_WINDOW_HPP
