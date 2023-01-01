#ifndef GRAPHICS_RENDERER_RENDERER_HPP
#define GRAPHICS_RENDERER_RENDERER_HPP

#include "vulkan/vulkan.h"

namespace graphics::renderer {

	class Renderer final {
	public:
		// Initialize the renderer's systems
		bool initialize();

		// Prepare for drawing
		bool update();

		// Draw the scene
		void render() const;

		// Deallocate resources
		void destroy();

	private:
		VkInstance instance;
		VkPhysicalDevice physicalDevice;

#ifndef NDEBUG
		VkDebugUtilsMessengerEXT debugMessenger;
#endif
	};

}

#endif // !GRAPHICS_RENDERER_RENDERER_HPP
