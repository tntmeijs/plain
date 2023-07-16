#include "graphics/renderer/shader_module.hpp"

#include "spdlog/spdlog.h"

#include "shaderc/shaderc.hpp"

#include <filesystem>
#include <fstream>
#include <ios>
#include <map>
#include <sstream>
#include <utility>

using namespace graphics::renderer;

static const std::map<std::string_view, shaderc_shader_kind> FILE_EXTENSION_TO_SHADER_TYPE {
	{ "vs", shaderc_vertex_shader },
	{ "fs", shaderc_fragment_shader }
};

ShaderModule::ShaderModule(const VkDevice& device) :
	device(device),
	spirV{},
	shaderModule(VK_NULL_HANDLE) {}

ShaderModule::ShaderModule(const ShaderModule& other):
	device(other.device),
	spirV(other.spirV),
	shaderModule(other.shaderModule)
{}

ShaderModule::ShaderModule(ShaderModule&& other) noexcept:
	device(other.device),
	spirV(std::move(other.spirV)),
	shaderModule(std::move(other.shaderModule))
{}

ShaderModule::~ShaderModule() {
	vkDestroyShaderModule(device, shaderModule, nullptr);
}

bool ShaderModule::compileFromFile(const std::string_view path) {
	if (!std::filesystem::exists(path)) {
		spdlog::error("Shader file not found: {}", path);
		spdlog::debug("Current working directory: {}", std::filesystem::current_path().string());
		return false;
	}

	shaderc::Compiler compiler {};
	shaderc::CompileOptions compileOptions {};

#ifdef NDEBUG
	compileOptions.SetOptimizationLevel(shaderc_optimization_level_performance);
#else
	compileOptions.SetOptimizationLevel(shaderc_optimization_level_zero);
#endif

	auto shaderType = shaderc_glsl_infer_from_source;

	const auto fileExtensionStartIndex = path.rfind('.');

	if (fileExtensionStartIndex == std::string::npos) {
		spdlog::warn("Unable to determine shader type because no file extension was found - shader type will be inferred from source");
	} else {
		const auto extension = path.substr(fileExtensionStartIndex + 1);

		auto found = FILE_EXTENSION_TO_SHADER_TYPE.find(extension);

		if (found != FILE_EXTENSION_TO_SHADER_TYPE.end()) {
			shaderType = found->second;
		} else {
			spdlog::error("Unrecognised shader file extension: {}", extension);
			return false;
		}
	}

	std::ifstream inputFile { path.data(), std::ios::binary };
	std::stringstream sourceCode {};
	sourceCode << inputFile.rdbuf();

	const auto compileResult = compiler.CompileGlslToSpv(sourceCode.str(), shaderType, path.data(), compileOptions);

	if (compileResult.GetCompilationStatus() != shaderc_compilation_status_success) {
		spdlog::error(compileResult.GetErrorMessage());
		return false;
	}

	spirV = { compileResult.begin(), compileResult.end() };

	spdlog::debug("Successfully compiled shader from: {}", path);
	return true;
}

bool ShaderModule::create() {
	if (shaderModule != VK_NULL_HANDLE) {
		spdlog::error("Cannot create shader module because a shader module exists already - please only create one shader module per shader");
		return false;
	}

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = spirV.size() * sizeof(std::uint32_t);
	createInfo.pCode = spirV.data();

	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		spdlog::error("Failed to create shader module");
		return false;
	}

	return true;
}

const VkShaderModule& ShaderModule::handle() const {
	return shaderModule;
}
