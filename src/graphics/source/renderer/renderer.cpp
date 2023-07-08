#include "graphics/renderer/renderer.hpp"
#include "graphics/window/window.hpp"

#include "spdlog/spdlog.h"

#include "GLFW/glfw3.h"

#include <algorithm>
#include <map>
#include <optional>
#include <vector>
#include <set>

using namespace graphics::renderer;
using namespace graphics::window;

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

Renderer::Renderer() :
	instance(VK_NULL_HANDLE),
	device(VK_NULL_HANDLE),
	graphicsQueue(VK_NULL_HANDLE),
	presentQueue(VK_NULL_HANDLE),
	surface(VK_NULL_HANDLE),
#ifndef NDEBUG
	debugMessenger(VK_NULL_HANDLE),
#endif
	isDestroyed(false) {}

bool Renderer::initialize(const window::Window& window) {
#ifndef NDEBUG
	VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
	debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugMessengerCreateInfo.pfnUserCallback = debugCallback;
	debugMessengerCreateInfo.pUserData = nullptr;
#endif

	const std::vector<const char*> requiredValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
	};

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
			const auto isMatchingExtensionName = [&requiredExtensionName](VkExtensionProperties properties) -> bool {
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
		std::uint32_t availableValidationLayerCount = 0;
		vkEnumerateInstanceLayerProperties(&availableValidationLayerCount, nullptr);

		std::vector<VkLayerProperties> availableValidationLayers(availableValidationLayerCount);
		vkEnumerateInstanceLayerProperties(&availableValidationLayerCount, availableValidationLayers.data());

		auto validationLayerIsMissing = false;
		spdlog::debug("Required validation layers:");
		for (const auto& requiredValidationLayerName : requiredValidationLayers) {
			const auto isMatchingValidationLayerName = [&requiredValidationLayerName](VkLayerProperties properties) -> bool {
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

	// Surface creation
	{
		if (glfwCreateWindowSurface(instance, window.getRawHandle(), nullptr, &surface) != VK_SUCCESS) {
			spdlog::error("Failed to create Vulkan surface");
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

	// Physical and logical device selection
	{
		std::uint32_t gpuCount = 0;
		vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);

		if (gpuCount == 0) {
			spdlog::error("No GPU with Vulkan support was found on this system");
		}

		std::vector<VkPhysicalDevice> gpus(gpuCount);
		vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.data());

		struct QueueFamilyIndices {
			std::optional<std::uint32_t> graphics;
			std::optional<std::uint32_t> present;

			/**
			 * @brief Utility method to help determine if all queue family indices were found
			 * @return True if all indices have been found, false when one or multiple indices are missing
			*/
			const bool isComplete() const {
				return graphics.has_value() && present.has_value();
			}

			/*
			* @brief Utility method to help construct a collection of all unique queue indices
			* @return Vector containing the unique queue indices
			*/
			const std::set<std::uint32_t> getUniqueIndices() const {
				return { graphics.value(), present.value() };
			}
		};

		const auto findQueueFamilyIndices = [](const VkPhysicalDevice& device, const VkSurfaceKHR& surface) -> QueueFamilyIndices {
			QueueFamilyIndices indices;

			std::uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

			std::uint32_t index = 0;
			for (const auto& queueFamily : queueFamilies) {
				if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) > 0) {
					indices.graphics = index;
				}

				VkBool32 hasPresentSupport = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &hasPresentSupport);

				if (hasPresentSupport == VK_TRUE) {
					indices.present = index;
				}

				// All necessary indices found, no need to keep searching
				if (indices.isComplete()) {
					break;
				}

				++index;
			}

			return indices;
		};

		const auto isDeviceSuitable = [&findQueueFamilyIndices](const VkPhysicalDevice& gpu, const VkSurfaceKHR& surface) -> bool {
			const auto queueFamilyIndices = findQueueFamilyIndices(gpu, surface);

			if (!queueFamilyIndices.isComplete()) {
				spdlog::trace("Unable to find all required queue family indices");
				return false;
			}

			return true;
		};

		const auto getTotalVideoRamOfDeviceInGigaBytes = [](const VkPhysicalDeviceMemoryProperties& properties) -> std::uint64_t {
			std::uint64_t total = 0;

			for (std::uint32_t i = 0; i < properties.memoryHeapCount; ++i) {
				const auto heap = properties.memoryHeaps[i];

				if ((heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == 0) {
					total += heap.size;
				}
			}

			// Bytes to gigabytes (rounded)
			return total / 1'000'000'000;
		};

		const auto rateGpu = [&getTotalVideoRamOfDeviceInGigaBytes](
			const VkPhysicalDeviceProperties& deviceProperties,
			const VkPhysicalDeviceFeatures& deviceFeatures,
			const VkPhysicalDeviceMemoryProperties& deviceMemoryProperties) -> std::uint64_t {
				// Suppress "unreferenced formal parameter"
				deviceFeatures;

				std::uint64_t score = 0;

				// Prefer discrete GPUs over integrated GPUs
				if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
					// Discrete GPUs are superior and should be preferred whenever possible
					score += 1'000'000;
				} else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
					// Integrated GPUs are not great, but work for applications that do not require lots of compute power
					score += 1'000;
				} else {
					// Any other "GPU" is probably pretty awful, but at least it supports Vulkan...
					score += 1;
				}

				// Prefer the device with the highest amount of video memory
				return score + getTotalVideoRamOfDeviceInGigaBytes(deviceMemoryProperties);
		};

		std::multimap<std::uint64_t, VkPhysicalDevice> ratingToGpuMapping;

		spdlog::debug("Found {} GPU{}:", gpuCount, gpuCount != 1 ? "s" : "");
		for (const auto& gpu : gpus) {
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;
			VkPhysicalDeviceMemoryProperties deviceMemoryProperties;

			vkGetPhysicalDeviceProperties(gpu, &deviceProperties);
			vkGetPhysicalDeviceFeatures(gpu, &deviceFeatures);
			vkGetPhysicalDeviceMemoryProperties(gpu, &deviceMemoryProperties);

			if (!isDeviceSuitable(gpu, surface)) {
				spdlog::debug("  {} [NOT SUITABLE]", deviceProperties.deviceName);
				continue;
			}

			spdlog::debug("  {} {}GB [OK]", deviceProperties.deviceName, getTotalVideoRamOfDeviceInGigaBytes(deviceMemoryProperties));

			ratingToGpuMapping.insert({ rateGpu(deviceProperties, deviceFeatures, deviceMemoryProperties), gpu });
		}

		// A multi-map internally sorts by key in ascending order, which makes them great for determing the best score
		const auto bestGpu = ratingToGpuMapping.rbegin();

		if (bestGpu == ratingToGpuMapping.rend()) {
			spdlog::error("No suitable physical device found");
			return false;
		}

		VkPhysicalDevice physicalDevice{ bestGpu->second };

		const auto queueFamilyIndices = findQueueFamilyIndices(physicalDevice, surface);

		// Should never happen if the device's suitability check includes a check that ensures the correct queue family indices are present
		if (!queueFamilyIndices.isComplete()) {
			spdlog::error("A suitable physical device was found, yet the application failed to find all queue family indices");
			return false;
		}

		constexpr float queuePriority{ 1.0f };
		const auto queuesToCreate = queueFamilyIndices.getUniqueIndices();

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		queueCreateInfos.reserve(queuesToCreate.size());

		for (const auto queueToCreate : queuesToCreate) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueToCreate;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

#ifndef NDEBUG
		deviceCreateInfo.enabledLayerCount = static_cast<std::uint32_t>(requiredValidationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = requiredValidationLayers.data();
#else
		deviceCreateInfo.enabledLayerCount = 0;
#endif

		if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
			spdlog::error("Failed to create Vulkan logical device");
			return false;
		}

		vkGetDeviceQueue(device, queueFamilyIndices.graphics.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, queueFamilyIndices.present.value(), 0, &presentQueue);
	}

	spdlog::debug("Renderer initialised");
	return true;
}

bool Renderer::update() {
	return false;
}

void Renderer::render() const {}

void Renderer::destroy() {
	if (isDestroyed) {
		spdlog::debug("Renderer already destroyed, this invocation will be ignored");
		return;
	}

	spdlog::debug("Destroying renderer");
	isDestroyed = true;

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

	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	spdlog::debug("Renderer destroyed");
}
