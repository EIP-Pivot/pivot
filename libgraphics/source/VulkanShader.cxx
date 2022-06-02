#include "pivot/graphics/VulkanShader.hxx"

#include "pivot/graphics/vk_utils.hxx"

#include <Logger.hpp>
#include <fstream>
constexpr static auto shaderStageToShaderC(const std::string &stage)
{
    if (stage == ".vert") return shaderc_vertex_shader;
    if (stage == ".frag") return shaderc_fragment_shader;
    if (stage == ".comp") return shaderc_compute_shader;
    throw std::runtime_error("Unknown shader type");
}

namespace pivot::graphics
{

VulkanShader::VulkanShader(const std::filesystem::path &path): name(path.stem().string()), shaderPath(path)
{
    reload();
}

VulkanShader::VulkanShader(const std::filesystem::path &path, const std::string &code)
    : name(path.stem().string()), shaderPath(path), source_code(code)
{
}

VulkanShader::VulkanShader(const std::filesystem::path &path, const std::vector<std::uint32_t> &byte_code)
    : name(path.stem().string()), shaderPath(path), byte_code(byte_code)
{
}

VulkanShader::~VulkanShader() {}

VulkanShader &VulkanShader::reload()
{
    source_code = vk_utils::readFile(shaderPath);
    return *this;
}

std::string VulkanShader::pre_process(shaderc::CompileOptions options)
{
    static shaderc::Compiler compiler;

    const auto preProcessResult = compiler.PreprocessGlsl(
        source_code, shaderStageToShaderC(shaderPath.extension().string()), getName().c_str(), options);
    if (preProcessResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        logger.err("Vulkan Shader/PreProcess") << "Failed to pre-process " << shaderPath << ": ";
        logger.err("Vulkan Shader/PreProcess") << preProcessResult.GetErrorMessage();
        throw VulkanShaderError("Failed to pre-process");
    }
    return std::string(preProcessResult.begin(), preProcessResult.end());
}

void VulkanShader::compile(VulkanShader::VulkanVersion version, VulkanShader::OptimizationLevel level)
{
    static shaderc::Compiler compiler;

    const auto options = getCompileOptions(version, level);
    const auto preprocess = pre_process(options);
    const auto filename = shaderPath.filename().string();
    const auto result = compiler.CompileGlslToSpv(preprocess, shaderStageToShaderC(shaderPath.extension().string()),
                                                  getName().c_str(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        logger.err("Vulkan Shader/Compile") << "While compiling shader file: " << shaderPath << ": ";
        logger.err("Vulkan Shader/Compile") << result.GetErrorMessage();
        throw VulkanShaderError("Failed to compile");
    }
    byte_code = std::vector<std::uint32_t>(result.cbegin(), result.cend());
}

shaderc::CompileOptions VulkanShader::getCompileOptions(VulkanVersion version, OptimizationLevel level)
{
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_vulkan,
                                 static_cast<decltype(shaderc_env_version_vulkan_1_0)>(version));
    switch (level) {
        case OptimizationLevel::None: options.SetGenerateDebugInfo(); break;
        case OptimizationLevel::Size: options.SetOptimizationLevel(shaderc_optimization_level_size); break;
        case OptimizationLevel::Performance:
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
            break;
    }
    for (const auto &[key, value]: macros) options.AddMacroDefinition(key, value);
    return options;
}

}    // namespace pivot::graphics
