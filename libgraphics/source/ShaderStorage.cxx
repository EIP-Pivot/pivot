#include "pivot/graphics/ShaderStorage.hxx"

#include "pivot/graphics/vk_utils.hxx"

using namespace std::literals;

namespace pivot::graphics
{

ShaderStorage::ShaderStorage() { std::filesystem::create_directory(cache_path); }

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

void ShaderStorage::load(const std::filesystem::path &path, const bool bForceCompile)
{
    if (path.extension() == ".spv") {
        logger.warn("Shader Storage/load") << "Attempting to load an already compiled shader. Will be imported as is, "
                                              "and won't be able to be rebuild.";
        shaderStorage.emplace(path.stem(), VulkanShader(path));
    }
    auto res = getCacheBinary(path, bForceCompile);
    if (res.has_value()) {
        shaderStorage.emplace(path.stem(), res.value());
    } else {
        try {
            shaderStorage.emplace(path.stem(), compileAndCache(path));
        } catch (const VulkanShader::VulkanShaderError &vse) {
            logger.err("Shader Storage/load") << vse.what();
            return;
        }
    }
}

std::optional<VulkanShader> ShaderStorage::getCacheBinary(const std::filesystem::path &path, const bool bForceCompile)
{
    const auto filename = path.filename();
    const std::filesystem::path cached_path = cache_path / (filename.string() + ".spv");

    if (!std::filesystem::exists(cached_path) && !bForceCompile) return std::nullopt;
    return VulkanShader(path, vk_utils::readBinaryFile<std::uint32_t>(cached_path));
}

VulkanShader ShaderStorage::compileAndCache(const std::filesystem::path &path)
{
    const std::filesystem::path true_path = shader_path / path;
    const std::filesystem::path cached_path = cache_path / (path.filename().string() + ".spv");

    VulkanShader shader(true_path);
    shader.compile(vulkanVersion, optimizationLevel);
    vk_utils::writeBinaryFile(cached_path, shader.getByteCode());
    return shader;
}

}    // namespace pivot::graphics
