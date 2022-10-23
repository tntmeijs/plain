#include "graphics/window/window.hpp"

#include "spdlog/spdlog.h"
#include "GLFW/glfw3.h"

using namespace graphics::window;

Window::Window(const std::uint32_t width, const std::uint32_t height, const std::string_view& title) :
	width(width),
	height(height),
	title(title),
	handle(nullptr) {
	if (glfwInit() != GLFW_TRUE) {
		spdlog::critical("Unable to initialize GLFW: {}", glfwGetError(nullptr));
	} else {
		spdlog::trace("GLFW initialized");
	}
}

Window::~Window() {
	glfwTerminate();
	spdlog::trace("GLFW terminated");
}

bool Window::create() {
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

	spdlog::info("Window created successfully");
	spdlog::info("    width: {}", width);
	spdlog::info("    height:{}", height);
	return handle != nullptr;
}

bool Window::isAlive() {
	return glfwWindowShouldClose(handle) == GLFW_FALSE;
}

void Window::poll() const {
	glfwPollEvents();
}

void Window::destroy() {
	if (handle != nullptr) {
		spdlog::trace("Destroyed GLFW window");
		glfwDestroyWindow(handle);
		handle = nullptr;
	}
}
