#ifndef GRAPHICS_WINDOW_WINDOW_HPP
#define GRAPHICS_WINDOW_WINDOW_HPP

#include <cstdint>
#include <string>
#include <string_view>

struct GLFWwindow;

namespace graphics::window {

	class Window final {
	public:
		Window(const std::uint32_t width, const std::uint32_t height, const std::string_view& title);
		Window(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(const Window&) = delete;
		Window& operator=(Window&&) = delete;
		~Window();

		// Create a new window
		bool create();

		// Returns whether this window is active / open
		bool isAlive();

		// Poll for user input
		void poll() const;

		// Destroy the window
		void destroy();

		// Get the raw window handle
		GLFWwindow* const getRawHandle() const;

	private:
		std::uint32_t width;
		std::uint32_t height;

		std::string title;

		GLFWwindow* handle;
	};

}

#endif // !GRAPHICS_WINDOW_WINDOW_HPP
