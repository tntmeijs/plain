#include "graphics/renderer/renderer.hpp"
#include "graphics/window/window.hpp"

#include "spdlog/spdlog.h"

using namespace graphics::window;
using namespace graphics::renderer;

constexpr const char* const USER_AGENT_NAME = "Plain/0.1";

int main(int argc, char* argv[]) {
	// Unreferenced formal parameter warnings
	argc;
	argv;

	spdlog::set_level(spdlog::level::level_enum::debug);

	auto window = Window(800, 600, "Plain - a webbrowser by Tahar Meijs");
	if (!window.create()) {
		spdlog::critical("Application failed to start because the window could not be created");
		return EXIT_FAILURE;
	}

	auto renderer = Renderer();
	renderer.initialize(window);

	do {
		window.poll();
		renderer.update();
		renderer.render();
	} while (window.isAlive());
	
	renderer.destroy();
	window.destroy();

	return EXIT_SUCCESS;
}
