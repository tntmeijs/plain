#include "graphics/renderer/renderer.hpp"

#include "spdlog/spdlog.h"

#include "GLFW/glfw3.h"

#include <algorithm>
#include <vector>

using namespace graphics::renderer;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
	void* userData) {
	// Suppress "unreferenced formal parameter" warning
	messageType;
	userData;

	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		spdlog::error(callbackData->pMessage);
	} else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		spdlog::warn(callbackData->pMessage);
	} else {
		spdlog::trace(callbackData->pMessage);
	}

	return VK_FALSE;
}

bool Renderer::initialize() {
#ifndef NDEBUG
	VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
	debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugMessengerCreateInfo.pfnUserCallback = debugCallback;
	debugMessengerCreateInfo.pUserData = nullptr;
#endif

	// Instance creation
	{
		std::uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> requiredExtensions(glfwExtensionCount);
		for (std::uint32_t i = 0; i < glfwExtensionCount; ++i) {
			requiredExtensions[i] = glfwExtensions[i];
		}

		requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

#ifndef NDEBUG
		requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		std::uint32_t availableExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

		auto extensionIsMissing = false;
		spdlog::debug("Required extensions:");
		for (const auto& requiredExtensionName : requiredExtensions) {
			const auto isMatchingExtensionName = [&requiredExtensionName](VkExtensionProperties properties) {
				return std::strcmp(properties.extensionName, requiredExtensionName) == 0;
			};

			const auto found = std::find_if(std::begin(availableExtensions), std::end(availableExtensions), isMatchingExtensionName) != std::end(availableExtensions);
			spdlog::debug("  [{}] {}", found ? "OK" : "MISSING", requiredExtensionName);

			if (!found) {
				extensionIsMissing = true;
			}
		}

		if (extensionIsMissing) {
			spdlog::error("One or multiple required Vulkan extensions are missing");
			return false;
		}

#ifndef NDEBUG
		const std::vector<const char*> requiredValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		std::uint32_t availableValidationLayerCount = 0;
		vkEnumerateInstanceLayerProperties(&availableValidationLayerCount, nullptr);

		std::vector<VkLayerProperties> availableValidationLayers(availableValidationLayerCount);
		vkEnumerateInstanceLayerProperties(&availableValidationLayerCount, availableValidationLayers.data());

		auto validationLayerIsMissing = false;
		spdlog::debug("Required validation layers:");
		for (const auto& requiredValidationLayerName : requiredValidationLayers) {
			const auto isMatchingValidationLayerName = [&requiredValidationLayerName](VkLayerProperties properties) {
				return std::strcmp(properties.layerName, requiredValidationLayerName) == 0;
			};

			const auto found = std::find_if(std::begin(availableValidationLayers), std::end(availableValidationLayers), isMatchingValidationLayerName) != std::end(availableValidationLayers);
			spdlog::debug("  [{}] {}", found ? "OK" : "MISSING", requiredValidationLayerName);

			if (!found) {
				validationLayerIsMissing = true;
			}
		}

		if (validationLayerIsMissing) {
			spdlog::error("One or multiple required Vulkan validation layers are missing");
			return false;
		}
#endif

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Plain Webbrowser";
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.pEngineName = "Plain";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &appInfo;
		instanceCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(requiredExtensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();
		instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

#ifdef NDEBUG
		instanceCreateInfo.enabledLayerCount = 0;
		instanceCreateInfo.pNext = nullptr;
#else
		instanceCreateInfo.enabledLayerCount = static_cast<std::uint32_t>(requiredValidationLayers.size());
		instanceCreateInfo.ppEnabledLayerNames = requiredValidationLayers.data();
		instanceCreateInfo.pNext = &debugMessengerCreateInfo;
#endif

		if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
			spdlog::error("Failed to create Vulkan instance");
			return false;
		}
	}

#ifndef NDEBUG
	// Debug messenger function set-up
	{
		const auto createDebugMessengerFunc = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

		if (createDebugMessengerFunc == nullptr) {
			spdlog::error("Failed to load \"vkCreateDebugUtilsMessengerEXT\" extension function");
			return false;
		}

		if (createDebugMessengerFunc(instance, &debugMessengerCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			spdlog::error("Unable to create debug messenger");
			return false;
		}
	}
#endif

	spdlog::debug("Renderer initialized");
	return true;
}

bool Renderer::update() {
	return false;
}

void Renderer::render() const {}

void Renderer::destroy() {
#ifndef NDEBUG
	// Debug messenger
	{
		auto destroyDebugMessengerFunc = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

		if (destroyDebugMessengerFunc != nullptr) {
			destroyDebugMessengerFunc(instance, debugMessenger, nullptr);
		} else {
			spdlog::error("Unable to destroy debug messenger because the extension function could not be loaded");
		}
	}
#endif

	vkDestroyInstance(instance, nullptr);
	spdlog::debug("Renderer destroyed");
}
