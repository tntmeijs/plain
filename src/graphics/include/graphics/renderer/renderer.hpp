#ifndef GRAPHICS_RENDERER_RENDERER_HPP
#define GRAPHICS_RENDERER_RENDERER_HPP

#include "graphics/window/window.hpp"

#include "vulkan/vulkan.h"

namespace graphics {

	namespace window {
		class Window;
	}

	namespace renderer {

		class Renderer final {
		public:
			Renderer();
			Renderer(const Renderer&) = delete;
			Renderer(Renderer&&) = delete;
			Renderer& operator=(const Renderer&) = delete;
			Renderer& operator=(const Renderer&&) = delete;
			~Renderer() = default;

			// Initialize the renderer's systems
			bool initialize(const window::Window& window);

			// Prepare for drawing
			bool update();

			// Draw the scene
			void render() const;

			// Deallocate resources
			void destroy();

		private:
			VkInstance instance;
			VkDevice device;
			VkQueue graphicsQueue;
			VkQueue presentQueue;
			VkSurfaceKHR surface;

#ifndef NDEBUG
			VkDebugUtilsMessengerEXT debugMessenger;
#endif

			bool isDestroyed;
		};

	}

}

#endif // !GRAPHICS_RENDERER_RENDERER_HPP
