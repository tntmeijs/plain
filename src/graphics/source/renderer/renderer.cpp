#include "graphics/renderer/renderer.hpp"
#include "graphics/renderer/shader_module.hpp"
#include "graphics/window/window.hpp"

#include "spdlog/spdlog.h"

#include "GLFW/glfw3.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <map>
#include <optional>
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
	instance{},
	device{},
	graphicsQueue{},
	presentQueue{},
	surface{},
	swapchain{},
	renderPass{},
	pipelineLayout{},
	graphicsPipeline{},
	commandPool{},
	commandBuffer{},
#ifndef NDEBUG
	debugMessenger{},
#endif
	swapchainImageFormat{ VK_FORMAT_UNDEFINED },
	swapchainExtent{ 0, 0 },
	swapchainImages{},
	swapchainImageViews{},
	swapchainFrameBuffers{},
	imageAvailableSemaphore{},
	renderFinishedSemaphore{},
	inFlightFence{},
	isDestroyed{ false } {}

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

	const std::vector<const char*> requiredDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

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
	spdlog::debug("Required instance extensions:");
	for (const auto& requiredExtensionName : requiredExtensions) {
		const auto isMatchingExtensionName = [&requiredExtensionName](VkExtensionProperties properties) -> bool {
			return std::strcmp(properties.extensionName, requiredExtensionName) == 0;
		};

		const auto found = std::find_if(availableExtensions.begin(), availableExtensions.end(), isMatchingExtensionName) != availableExtensions.end();
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

		const auto found = std::find_if(availableValidationLayers.begin(), availableValidationLayers.end(), isMatchingValidationLayerName) != availableValidationLayers.end();
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

	if (glfwCreateWindowSurface(instance, window.getRawHandle(), nullptr, &surface) != VK_SUCCESS) {
		spdlog::error("Failed to create Vulkan surface");
		return false;
	}

#ifndef NDEBUG
	const auto createDebugMessengerFunc = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

	if (createDebugMessengerFunc == nullptr) {
		spdlog::error("Failed to load \"vkCreateDebugUtilsMessengerEXT\" extension function");
		return false;
	}

	if (createDebugMessengerFunc(instance, &debugMessengerCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		spdlog::error("Unable to create debug messenger");
		return false;
	}
#endif

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

	constexpr auto findQueueFamilyIndices = [](const VkPhysicalDevice& device, const VkSurfaceKHR& surface) -> QueueFamilyIndices {
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

	struct SwapchainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	constexpr auto querySwapchainSupport = [](const VkPhysicalDevice& gpu, const VkSurfaceKHR& surface) {
		SwapchainSupportDetails details{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &details.capabilities);

		std::uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, details.formats.data());
		}

		std::uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	};

	const auto isDeviceSuitable = [&findQueueFamilyIndices, &requiredDeviceExtensions, &querySwapchainSupport](const VkPhysicalDevice& gpu, const VkSurfaceKHR& surface) -> bool {
		std::uint32_t deviceExtensionCount = 0;
		vkEnumerateDeviceExtensionProperties(gpu, nullptr, &deviceExtensionCount, nullptr);

		std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
		vkEnumerateDeviceExtensionProperties(gpu, nullptr, &deviceExtensionCount, deviceExtensions.data());

		std::uint32_t foundRequiredExtensionCount = 0;
		spdlog::debug("    Required device extensions:");
		for (const auto& requiredExtensionName : requiredDeviceExtensions) {
			const auto isMatchingExtensionName = [&requiredExtensionName](const VkExtensionProperties& properties) {
				return std::strcmp(properties.extensionName, requiredExtensionName) == 0;
			};

			bool found = false;
			if (std::find_if(deviceExtensions.begin(), deviceExtensions.end(), isMatchingExtensionName) != deviceExtensions.end()) {
				++foundRequiredExtensionCount;
				found = true;
			}

			spdlog::debug("      [{}] {}", found ? "OK" : "MISSING", requiredExtensionName);
		}

		if (foundRequiredExtensionCount != requiredDeviceExtensions.size()) {
			spdlog::trace("Unable to find all required device extensions");
			return false;
		}

		const auto swapchainSupport = querySwapchainSupport(gpu, surface);
		if (swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty()) {
			spdlog::trace("Unable to find a swap chain with the necessary capabilities");
			return false;
		}

		const auto queueFamilyIndices = findQueueFamilyIndices(gpu, surface);

		if (!queueFamilyIndices.isComplete()) {
			spdlog::trace("Unable to find all required queue family indices");
			return false;
		}

		return true;
	};

	constexpr auto getTotalVideoRamOfDeviceInGigaBytes = [](const VkPhysicalDeviceMemoryProperties& properties) -> std::uint64_t {
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

		spdlog::debug("  {} {}GB", deviceProperties.deviceName, getTotalVideoRamOfDeviceInGigaBytes(deviceMemoryProperties));

		if (!isDeviceSuitable(gpu, surface)) {
			spdlog::debug("    Device is unsuitable for use");
			continue;
		}

		spdlog::debug("    Device is suitable for use");

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
	deviceCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(requiredDeviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();

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

	constexpr auto chooseSwapchainSurfaceFormat = [](const std::vector<VkSurfaceFormatKHR>& availableFormats) -> VkSurfaceFormatKHR {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				// Prefer the most accurate format, which is non-linear sRGB
				return availableFormat;
			}
		}

		// If the preferred format does not exist, we will just use whatever is available first
		return availableFormats[0];
	};

	constexpr auto chooseSwapchainPresentMode = [](const std::vector<VkPresentModeKHR>& availablePresentModes) -> VkPresentModeKHR {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				// Prefer mailbox present mode to allow for triple-buffering, low latency, and reduced tearing
				// If running on a mobile device, FIFO might be preferable instead due to the energy usage of the mailbox present mode
				return availablePresentMode;
			}
		}

		// FIFO is always available as per the Vulkan specification
		return VK_PRESENT_MODE_FIFO_KHR;
	};

	const auto chooseSwapchainExtent = [&window](const VkSurfaceCapabilitiesKHR& surfaceCapabilities) -> VkExtent2D {
		if (surfaceCapabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
			return surfaceCapabilities.currentExtent;
		}

		// Special value was set by window manager - try to make a framebuffer with a resolution that best matches the window
		// The window's resolution will be the target size, unless it exceeds the bounds of the surface
		return {
			std::clamp(window.getWidth(), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width),
			std::clamp(window.getHeight(), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height)
		};
	};

	// Create a swapchain
	const auto swapchainSupport = querySwapchainSupport(physicalDevice, surface);
	const auto swapchainSurfaceFormat = chooseSwapchainSurfaceFormat(swapchainSupport.formats);
	const auto swapchainPresentMode = chooseSwapchainPresentMode(swapchainSupport.presentModes);

	swapchainImageFormat = swapchainSurfaceFormat.format;
	swapchainExtent = chooseSwapchainExtent(swapchainSupport.capabilities);

	std::uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
	imageCount = std::clamp(imageCount, imageCount, swapchainSupport.capabilities.maxImageCount);

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageFormat = swapchainSurfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = swapchainSurfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = swapchainExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = swapchainPresentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	if (queueFamilyIndices.graphics.value() != queueFamilyIndices.present.value()) {
		const std::uint32_t indices[] = { queueFamilyIndices.graphics.value(), queueFamilyIndices.present.value() };

		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = indices;
	} else {
		// To keep things simple, if both queue families are identical, exclusive mode will be used to avoid having to deal with ownership
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS) {
		spdlog::error("Failed to create swapchain");
		return false;
	}

	// Retrieve images that back the swapchain
	std::uint32_t swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr);

	swapchainImages.resize(swapchainImageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data());

	swapchainImageViews.resize(swapchainImages.size());

	for (auto i = 0; i < swapchainImageViews.size(); ++i) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapchainImageFormat;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		createInfo.components = {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};

		if (vkCreateImageView(device, &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
			spdlog::error("Failed to create image view for swapchain image");
			return false;
		}
	}

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	// Protect against trying to perform a subpass before an image is available
	// This ensures that the implicit subpass before the explclit subpass is handled correctly
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachment;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
		spdlog::error("Failed to create render pass");
		return false;
	}

	// Load shader source code and compile into SPIR-V
	auto vertexShaderModule = ShaderModule{ device };

	if (!vertexShaderModule.compileFromFile("./resources/shaders/triangle.vs") || !vertexShaderModule.create()) {
		return false;
	}

	VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageInfo.module = vertexShaderModule.handle();
	vertexShaderStageInfo.pName = "main";

	auto fragmentShaderModule = ShaderModule{ device };

	if (!fragmentShaderModule.compileFromFile("./resources/shaders/triangle.fs") || !fragmentShaderModule.create()) {
		return false;
	}

	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
	fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageInfo.module = fragmentShaderModule.handle();
	fragmentShaderStageInfo.pName = "main";

	const std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vertexShaderStageInfo, fragmentShaderStageInfo };

	// Pipeline creation
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineVertexInputStateCreateInfo vertexInput{};
	vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.vertexBindingDescriptionCount = 0;
	vertexInput.vertexAttributeDescriptionCount = 0;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		spdlog::error("Failed to create pipeline layout");
		return false;
	}

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = static_cast<std::uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = &vertexInput;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pColorBlendState = &colorBlending;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		spdlog::error("Failed to create graphics pipeline");
		return false;
	}

	swapchainFrameBuffers.resize(swapchainImageViews.size());

	for (auto i = 0; i < swapchainImageViews.size(); ++i) {
		VkImageView attachments[] = { swapchainImageViews[i] };

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = attachments;
		framebufferCreateInfo.width = swapchainExtent.width;
		framebufferCreateInfo.height = swapchainExtent.height;
		framebufferCreateInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &swapchainFrameBuffers[i]) != VK_SUCCESS) {
			spdlog::error("Failed to create framebuffer for index {}", i);
			return false;
		}
	}

	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphics.value();

	if (vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
		spdlog::error("Failed to create command pool");
		return false;
	}

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS) {
		spdlog::error("Failed to create command buffer");
		return false;
	}

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS || vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS) {
		spdlog::error("Failed to create semaphores");
		return false;
	}

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;	// Signaled upon creation so the first frame never waits for the fence that it should signal after the first frame renders

	if (vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
		spdlog::error("Failed to create fence");
		return false;
	}

	spdlog::debug("Renderer initialised");
	return true;
}

void Renderer::render() const {
	// Wait for previous frame to finish
	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
	vkResetFences(device, 1, &inFlightFence);

	// Acquire an image from the swapchain
	std::uint32_t swapchainImageIndex;
	vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<std::uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &swapchainImageIndex);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkClearValue clearColor{ 0.3921568f, 0.5843137f, 0.9294117f, 1.0f };

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapchainFrameBuffers[swapchainImageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapchainExtent;
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapchainExtent.width);
	viewport.height = static_cast<float>(swapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchainExtent;

	// Start recording rendering commands
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		spdlog::error("Failed to begin recording into the command buffer");
		return;
	}

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		spdlog::error("Failed to end recording into the command buffer");
		return;
	}

	// Submit the commands to the GPU
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

	// Signals the fence so the CPU will block at the start of this method until the GPU has finished execution
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
		spdlog::error("Failed to submit queue");
		return;
	}

	// Present the image to the screen
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &swapchainImageIndex;

	if (vkQueuePresentKHR(presentQueue, &presentInfo) != VK_SUCCESS) {
		spdlog::error("Failed to present");
	}
}

void Renderer::destroy() {
	if (isDestroyed) {
		spdlog::debug("Renderer already destroyed, this invocation will be ignored");
		return;
	}

	spdlog::debug("Destroying renderer");
	isDestroyed = true;

	vkDeviceWaitIdle(device);

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

	vkDestroyFence(device, inFlightFence, nullptr);

	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);

	vkDestroyCommandPool(device, commandPool, nullptr);

	for (const auto& framebuffer : swapchainFrameBuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	swapchainFrameBuffers.clear();

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

	for (const auto& swapchainImageView : swapchainImageViews) {
		vkDestroyImageView(device, swapchainImageView, nullptr);
	}

	swapchainImageViews.clear();

	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	spdlog::debug("Renderer destroyed");
}
