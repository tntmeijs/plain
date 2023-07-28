#ifndef GRAPHICS_RENDERER_RENDERER_HPP
#define GRAPHICS_RENDERER_RENDERER_HPP

#include "vulkan/vulkan.h"

#include <cstdint>
#include <vector>

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
			VkSwapchainKHR swapchain;
			VkRenderPass renderPass;
			VkPipelineLayout pipelineLayout;
			VkPipeline graphicsPipeline;
			VkCommandPool commandPool;
			VkCommandBuffer commandBuffer;

#ifndef NDEBUG
			VkDebugUtilsMessengerEXT debugMessenger;
#endif

			VkFormat swapchainImageFormat;
			VkExtent2D swapchainExtent;
			std::vector<VkImage> swapchainImages;
			std::vector<VkImageView> swapchainImageViews;
			std::vector<VkFramebuffer> swapchainFrameBuffers;
			std::uint32_t swapchainImageIndex;

			bool isDestroyed;
		};

	}

}

#endif // !GRAPHICS_RENDERER_RENDERER_HPP
