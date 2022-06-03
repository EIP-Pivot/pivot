#include "pivot/graphics/ShaderStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <spirv-tools/libspirv.hpp>

namespace pivot::graphics
{

constexpr spv_target_env convert(const VulkanShader::VulkanVersion &env)
{
    switch (env) {
        case VulkanShader::VulkanVersion::e1_0: return SPV_ENV_VULKAN_1_0;
        case VulkanShader::VulkanVersion::e1_1: return SPV_ENV_VULKAN_1_1;
        case VulkanShader::VulkanVersion::e1_2: return SPV_ENV_VULKAN_1_2;
        case VulkanShader::VulkanVersion::e1_3: return SPV_ENV_VULKAN_1_3;
    }
    throw std::logic_error("Unsupported target env");
};

ShaderStorage::ShaderStorage()
{
    logger.debug("ShaderStorage") << "Shader path: " << shader_path;
    logger.debug("ShaderStorage") << "Shader cache path: " << cache_path;
    std::filesystem::create_directory(cache_path);
}

ShaderStorage::~ShaderStorage() {}

void ShaderStorage::setOptions(const VulkanShader::VulkanVersion &ver, const VulkanShader::OptimizationLevel &op)
{
    optimizationLevel = op;
    vulkanVersion = ver;
    recompile();
}

void ShaderStorage::recompile(const std::string &name)
{
    shaderStorage.at(name).reload().compile(vulkanVersion, optimizationLevel);
}

void ShaderStorage::recompile()
{
    for (const auto &[name, _]: shaderStorage) { recompile(name); }
}

std::string ShaderStorage::load(const std::filesystem::path &path, const bool bForceCompile)
{
    DEBUG_FUNCTION
    const std::filesystem::path true_path = shader_path / path;
    const auto filename = path.filename().string();
    const spvtools::SpirvTools tools(convert(vulkanVersion));

    if (path.extension() == ".spv") {
        logger.warn("Shader Storage/load") << "Attempting to load an already compiled shader. Will be imported as is, "
                                              "and won't be able to be rebuild.";
        shaderStorage.emplace(filename, VulkanShader(true_path));
    } else {
        auto res = getCacheBinary(true_path, bForceCompile);
        if (res.has_value()) {
            if (!tools.Validate(res.value().getByteCode())) {
                logger.debug("Shader Storage/load")
                    << "Shader \"" << filename
                    << "\" found in cache, but the binary appear corrupted. It will be recompiled.";
                res = compileAndCache(true_path);
            } else {
                logger.debug("Shader Storage/load") << "Shader \"" << filename << "\" found in cache.";
            }
            shaderStorage.emplace(filename, res.value());
        } else {
            logger.debug("Shader Storage/load")
                << "Shader \"" << filename << "\" was not found in the cache! It will be compiled.";
            shaderStorage.emplace(filename, compileAndCache(true_path));
        }
    }
    assert(tools.Validate(shaderStorage.at(filename).getByteCode()));
    return path.filename().string();
}

std::optional<std::string> ShaderStorage::try_load(const std::filesystem::path &path, const bool bForceCompile)
try {
    return load(path, bForceCompile);
} catch (const VulkanShader::VulkanShaderError &vse) {
    logger.err(vse.getScope()) << "Failed to load a shader :" << vse.what();
    return std::nullopt;
}

std::optional<VulkanShader> ShaderStorage::getCacheBinary(const std::filesystem::path &path, const bool bForceCompile)
{
    DEBUG_FUNCTION
    const auto filename = path.filename();
    const std::filesystem::path cached_path = cache_path / (filename.string() + ".spv");

    if (!std::filesystem::exists(cached_path) || bForceCompile) return std::nullopt;
    return VulkanShader(path, vk_utils::readBinaryFile<std::uint32_t>(cached_path));
}

VulkanShader ShaderStorage::compileAndCache(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
    const std::filesystem::path cached_path = cache_path / (path.filename().string() + ".spv");

    VulkanShader shader(path);
    shader.compile(vulkanVersion, optimizationLevel);
    vk_utils::writeBinaryFile(cached_path, std::span(shader.getByteCode()));
    return shader;
}

}    // namespace pivot::graphics
