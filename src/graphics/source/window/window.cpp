#include "graphics/window/window.hpp"

#include "spdlog/spdlog.h"

using namespace graphics::window;

Window::Window() :
	handle(nullptr) {}

Window::~Window() {}

bool Window::create() {
	spdlog::info("Window created successfully");
	return true;
}
