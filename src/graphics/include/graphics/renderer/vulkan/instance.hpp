#ifndef GRAPHICS_RENDERER_VULKAN_INSTANCE_HPP
#define GRAPHICS_RENDERER_VULKAN_INSTANCE_HPP

#include "vulkan/vulkan.h"

#include <string>
#include <string_view>
#include <set>

namespace graphics::renderer::vulkan {

	class Instance {
	public:
		Instance();
		Instance(const Instance& other);
		Instance(Instance&& other) noexcept;
		Instance& operator=(const Instance& other);
		Instance& operator=(Instance&& other) noexcept;
		~Instance();

		/**
		 * Create a new {@link Instance} with an application name and optionally a debug messenger
		 * @param applicationName Name of the application
		 * @param isDebug True to enable debug messages and validation layers, false to disable it
		 * @return New {@link Instance}
		 */
		static Instance newBuilder(const std::string_view applicationName, const bool isDebug);

		/**
		 * Filters out all unsupported validation layers
		 * @return All unsupported validation layers, empty if all validation layers are supported
		 */
		const std::set<std::string_view> getMissingValidationLayers() const;

		/**
		 * Returns all validation layers added to the instance
		 * @return Validation layers
		 */
		const std::set<const char*>& getValidationLayers() const;

		/**
		 * Filters out all unsupported extensions
		 * @return All unsupported extensions, empty if all extensions are supported
		 */
		const std::set<std::string_view> getMissingExtensions() const;

		/**
		 * Returns all extensions added to the instance
		 * @return Extensions
		 */
		const std::set<const char*>& getExtensions() const;

		/**
		 * Add an extension to the instance
		 * @param extension Name of the extension
		 * @return Updated {@link Instance}
		 */
		Instance& addExtension(const std::string_view extension);

		/**
		 * Add validation layers to the instance
		 * @param validationLayer Name of the validation layer
		 * @return Updated {@link Instance}
		 */
		Instance& addValidationLayer(const std::string_view validationLayer);

		/**
		 * Add an application version
		 * @param major Semver's major component
		 * @param minor Semver's minor component
		 * @param patch Sember's patch component
		 * @return Updated {@link Instance}
		 */
		Instance& withApplicationVersion(const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch);

		/**
		 * Add an engine version
		 * @param major Semver's major component
		 * @param minor Semver's minor component
		 * @param patch Sember's patch component
		 * @return Updated {@link Instance}
		 */
		Instance& withEngineVersion(const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch);

		/**
		 * Create a new Vulkan instance
		 * @return True on success, false on failure
		 */
		bool create();

		/**
		 * Loads the debug messenger which allows for validation layer output to be intercepted
		 * @return True if the debug messenger was loaded successfully, false when not
		 */
		bool loadDebugMessenger();

		/**
		 * Retrieve a handle to the raw Vulkan object
		 * @return Reference to the {@link VkInstance} object that backs this wrapper
		 */
		const VkInstance& handle() const;

	private:
		/**
		 * Vulkan debug message callback
		 * @param messageSeverity Importance of a message (similar to a log level)
		 * @param messageTypes The type of data this message is about
		 * @param pCallbackData Data from the debug callback
		 * @return Result value of the debug callback as per the API specification
		 */
		VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) const;

		/**
		 * Utility method that helps configure the debug messenger
		 * @return Debug messenger create information
		 */
		const VkDebugUtilsMessengerCreateInfoEXT createDebugUtilsMessengerCreateInfoExt();

	private:
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;

		std::string applicationName;
		std::string engineName;

		std::uint32_t applicationVersion;
		std::uint32_t engineVersion;

		std::set<const char*> extensions;
		std::set<const char*> validationLayers;

		bool isDebug;
	};

}

#endif // !GRAPHICS_RENDERER_VULKAN_INSTANCE_HPP
