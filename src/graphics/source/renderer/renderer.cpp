#include "graphics/renderer/renderer.hpp"

#include "spdlog/spdlog.h"

#include "GLFW/glfw3.h"

#include <algorithm>
#include <vector>

using namespace graphics::renderer;

bool Renderer::initialize() {
	{
		std::uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> requiredExtensions(glfwExtensionCount);
		for (std::uint32_t i = 0; i < glfwExtensionCount; ++i) {
			requiredExtensions[i] = glfwExtensions[i];
		}

		requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

		std::uint32_t availableExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

		auto extensionIsMissing = false;
		spdlog::trace("Required extensions:");
		for (const auto& extensionName : requiredExtensions) {
			const auto isMatchingExtensionName = [&extensionName](VkExtensionProperties properties) {
				return std::strcmp(properties.extensionName, extensionName) == 0;
			};

			const auto found = std::find_if(availableExtensions.begin(), availableExtensions.end(), isMatchingExtensionName) != std::end(availableExtensions);
			spdlog::trace("  [{}] {}", found ? "OK" : "MISSING", extensionName);

			if (!found) {
				extensionIsMissing = true;
			}
		}

		if (extensionIsMissing) {
			spdlog::error("One or multiple required Vulkan extensions are missing");
			return false;
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Plain Webbrowser";
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.pEngineName = "Plain";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo instanceCreateInfo = {};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &appInfo;
		instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
		instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;
		instanceCreateInfo.enabledLayerCount = 0;
		instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

		if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
			spdlog::error("Failed to create Vulkan instance");
			return false;
		}
	}

	spdlog::trace("Renderer initialized");
	return true;
}

bool Renderer::update() {
	return false;
}

void Renderer::render() const {}

void Renderer::destroy() {
	vkDestroyInstance(instance, nullptr);
	spdlog::trace("Renderer destroyed");
}
