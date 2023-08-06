#include "graphics/renderer/vulkan/instance.hpp"

#include "spdlog/spdlog.h"

#include <cstdint>
#include <cstring>
#include <limits>
#include <utility>
#include <vector>

using namespace graphics::renderer::vulkan;

Instance::Instance() :
	instance{ VK_NULL_HANDLE },
	debugMessenger{ VK_NULL_HANDLE },
	applicationName{},
	engineName{},
	applicationVersion{ std::numeric_limits<std::uint32_t>::max() },
	engineVersion{ std::numeric_limits<std::uint32_t>::max() },
	extensions{},
	validationLayers{},
	isDebug{ false } {}

Instance::Instance(const Instance& other) :
	instance{ other.instance },
	debugMessenger{ other.debugMessenger },
	applicationName{ other.applicationName },
	engineName{ other.engineName },
	applicationVersion{ other.applicationVersion },
	engineVersion{ other.engineVersion },
	extensions{ other.extensions },
	validationLayers{ other.validationLayers },
	isDebug{ other.isDebug } {}

Instance::Instance(Instance&& other) noexcept :
	instance{ std::exchange(other.instance, VK_NULL_HANDLE) },
	debugMessenger{ std::exchange(other.debugMessenger, VK_NULL_HANDLE) },
	applicationName{ std::move(other.applicationName) },
	engineName{ std::move(other.engineName) },
	applicationVersion{ other.applicationVersion },
	engineVersion{ other.engineVersion },
	extensions{ std::move(other.extensions) },
	validationLayers{ std::move(other.validationLayers) },
	isDebug{ other.isDebug } {}

Instance& Instance::operator=(const Instance& other) {
	instance = other.instance;
	debugMessenger = other.debugMessenger;
	applicationName = other.applicationName;
	engineName = other.engineName;
	applicationVersion = other.applicationVersion;
	engineVersion = other.engineVersion;
	extensions = other.extensions;
	validationLayers = other.validationLayers;
	isDebug = other.isDebug;

	return *this;
}

Instance& Instance::operator=(Instance&& other) noexcept {
	instance = std::exchange(other.instance, VK_NULL_HANDLE);
	debugMessenger = std::exchange(other.debugMessenger, VK_NULL_HANDLE);
	applicationName = std::move(other.applicationName);
	engineName = std::move(other.engineName);
	applicationVersion = other.applicationVersion;
	engineVersion = other.engineVersion;
	extensions = std::move(other.extensions);
	validationLayers = std::move(other.validationLayers);
	isDebug = other.isDebug;

	return *this;
}

Instance::~Instance() {
	if (isDebug && debugMessenger != VK_NULL_HANDLE) {
		auto destroyDebugMessengerFunc = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

		if (destroyDebugMessengerFunc != nullptr) {
			destroyDebugMessengerFunc(instance, debugMessenger, nullptr);
		} else {
			spdlog::error("Unable to destroy debug messenger because the extension function could not be loaded");
		}
	}

	if (instance != VK_NULL_HANDLE) {
		vkDestroyInstance(instance, nullptr);
	}
}

Instance Instance::newBuilder(const std::string_view applicationName, const bool enableDebugMessenger) {
	Instance newInstance{};
	newInstance.applicationName = applicationName;
	newInstance.isDebug = enableDebugMessenger;

	return newInstance;
}

const std::set<std::string_view> Instance::getMissingValidationLayers() const {
	std::uint32_t availableValidationLayerCount;
	vkEnumerateInstanceLayerProperties(&availableValidationLayerCount, nullptr);

	std::vector<VkLayerProperties> availableValidationLayers(availableValidationLayerCount);
	vkEnumerateInstanceLayerProperties(&availableValidationLayerCount, availableValidationLayers.data());

	std::set<std::string_view> missingValidationLayers;

	for (const auto& requestedValidationLayer : validationLayers) {
		bool found = false;

		for (const auto& availableValidationLayer : availableValidationLayers) {
			if (std::strcmp(availableValidationLayer.layerName, requestedValidationLayer) == 0) {
				found = true;
				break;
			}
		}

		if (!found) {
			missingValidationLayers.insert(requestedValidationLayer);
		}
	}

	return missingValidationLayers;
}

const std::set<const char*>& graphics::renderer::vulkan::Instance::getValidationLayers() const {
	return validationLayers;
}

const std::set<std::string_view> Instance::getMissingExtensions() const {
	std::uint32_t availableExtensionCount;
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

	std::set<std::string_view> missingExtensions{};

	for (const auto& requestedExtension : extensions) {
		bool found = false;

		for (const auto& availableExtension : availableExtensions) {
			if (std::strcmp(availableExtension.extensionName, requestedExtension) == 0) {
				found = true;
				break;
			}
		}

		if (!found) {
			missingExtensions.insert(requestedExtension);
		}
	}

	return missingExtensions;
}

const std::set<const char*>& Instance::getExtensions() const {
	return extensions;
}

Instance& Instance::addExtension(const std::string_view extension) {
	extensions.insert(extension.data());
	return *this;
}

Instance& Instance::addValidationLayer(const std::string_view validationLayer) {
	validationLayers.insert(validationLayer.data());
	return *this;
}

Instance& Instance::withApplicationVersion(const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch) {
	applicationVersion = VK_MAKE_API_VERSION(0, major, minor, patch);
	return *this;
}

Instance& Instance::withEngineVersion(const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch) {
	engineVersion = VK_MAKE_API_VERSION(0, major, minor, patch);
	return *this;
}

bool Instance::create() {
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = applicationName.data();
	appInfo.apiVersion = VK_API_VERSION_1_0;

	appInfo.pEngineName = engineName.empty()
		? applicationName.data()
		: engineName.data();

	appInfo.applicationVersion = applicationVersion == std::numeric_limits<std::uint32_t>::max()
		? VK_MAKE_API_VERSION(0, 0, 0, 0)
		: applicationVersion;

	appInfo.engineVersion = engineVersion == std::numeric_limits<std::uint32_t>::max()
		? VK_MAKE_API_VERSION(0, 0, 0, 0)
		: engineVersion;

	std::vector<const char*> extensionsVector(extensions.begin(), extensions.end());
	std::vector<const char*> validationLayersVector(validationLayers.begin(), validationLayers.end());

	const auto debugCreateInfo = createDebugUtilsMessengerCreateInfoExt();

	VkInstanceCreateInfo instanceInfo{};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
	instanceInfo.ppEnabledExtensionNames = extensionsVector.data();

	if (isDebug) {
		instanceInfo.enabledLayerCount = static_cast<std::uint32_t>(validationLayers.size());
		instanceInfo.ppEnabledLayerNames = validationLayersVector.data();
		instanceInfo.pNext = &debugCreateInfo;
	}

	if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS) {
		spdlog::error("Failed to create Vulkan instance");
		return false;
	}

	return true;
}

bool Instance::loadDebugMessenger() {
	const auto createInfo = createDebugUtilsMessengerCreateInfoExt();
	const auto createDebugMessengerFunc = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

	if (createDebugMessengerFunc == nullptr) {
		spdlog::error("Failed to load \"vkCreateDebugUtilsMessengerEXT\" extension function");
		return false;
	}

	if (createDebugMessengerFunc(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		spdlog::error("Failed to create debug messenger");
		return false;
	}

	return true;
}

const VkInstance& Instance::handle() const {
	return instance;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Instance::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) const {
	// Suppress "unreferenced formal parameter" warning
	messageTypes;

	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		spdlog::error(pCallbackData->pMessage);
	} else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		spdlog::warn(pCallbackData->pMessage);
	} else {
		spdlog::trace(pCallbackData->pMessage);
	}

	return VK_FALSE;
}

const VkDebugUtilsMessengerCreateInfoEXT Instance::createDebugUtilsMessengerCreateInfoExt() {
	VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
	debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugMessengerCreateInfo.pUserData = this;
	debugMessengerCreateInfo.pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) -> VkBool32 {
		return static_cast<const Instance* const>(pUserData)->debugCallback(messageSeverity, messageTypes, pCallbackData);
	};

	debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	return debugMessengerCreateInfo;
}
