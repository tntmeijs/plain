#ifndef GRAPHICS_RENDERER_SHADER_MODULE_HPP
#define GRAPHICS_RENDERER_SHADER_MODULE_HPP

#include "vulkan/vulkan.h"

#include <cstdint>
#include <string_view>
#include <vector>

namespace graphics::renderer {

	class ShaderModule {
	public:
		ShaderModule() = delete;
		ShaderModule(const VkDevice& device);
		ShaderModule(const ShaderModule& other);
		ShaderModule(ShaderModule&& other) noexcept;
		ShaderModule& operator=(const ShaderModule& other) = delete;
		ShaderModule& operator=(ShaderModule&& other) noexcept = delete;
		~ShaderModule();

		// Compile the contents of a given source file into SPIR-V shader bytecode
		bool compileFromFile(const std::string_view source);

		// Turn the compiled shader bytecode into a Vulkan shader module
		bool create();

		// Access the raw handle to the Vulkan object
		const VkShaderModule& handle() const;

	private:
		std::vector<std::uint32_t> spirV;

		const VkDevice& device;
		VkShaderModule shaderModule;
	};

}

#endif // !GRAPHICS_RENDERER_SHADER_MODULE_HPP
