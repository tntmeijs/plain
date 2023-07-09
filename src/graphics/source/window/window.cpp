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
		spdlog::debug("GLFW initialised");
	}
}

Window::~Window() {
	glfwTerminate();
	spdlog::debug("GLFW terminated");
}

bool Window::create() {
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

	spdlog::debug("Window created successfully");
	spdlog::debug("    width:  {}", width);
	spdlog::debug("    height: {}", height);
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
		spdlog::debug("Destroyed GLFW window");
		glfwDestroyWindow(handle);
		handle = nullptr;
	}
}

GLFWwindow* const Window::getRawHandle() const {
	return handle;
}

std::uint32_t Window::getWidth() const {
	int widthValue = 0;
	glfwGetFramebufferSize(handle, &widthValue, nullptr);

	return static_cast<std::uint32_t>(widthValue);
}

std::uint32_t Window::getHeight() const {
	int heightValue = 0;
	glfwGetFramebufferSize(handle, nullptr, &heightValue);

	return static_cast<std::uint32_t>(heightValue);
}
