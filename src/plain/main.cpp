#include "graphics/renderer/renderer.hpp"
#include "graphics/window/window.hpp"

#include "spdlog/spdlog.h"

#include <cstdlib>

using namespace graphics::window;
using namespace graphics::renderer;

constexpr const char* const USER_AGENT_NAME = "Plain/0.1";
constexpr const char* const APPLICATION_NAME = "Plain";

int main(int argc, char* argv[]) {
	// Unreferenced formal parameter warnings
	argc;
	argv;

	spdlog::set_level(spdlog::level::level_enum::debug);

	Window window(800, 600, APPLICATION_NAME);
	if (!window.create()) {
		spdlog::critical("Application failed to start because the window could not be created");
		return EXIT_FAILURE;
	}

	Renderer renderer{};
	if (!renderer.initialize(APPLICATION_NAME, window)) {
		return EXIT_FAILURE;
	}

	do {
		window.poll();
		renderer.render();
	} while (window.isAlive());

	renderer.destroy();
	window.destroy();

	return EXIT_SUCCESS;
}
