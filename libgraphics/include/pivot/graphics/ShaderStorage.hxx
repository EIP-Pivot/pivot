#pragma once

#include "pivot/graphics/VulkanShader.hxx"

#include <filesystem>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#ifndef PIVOT_SHADER_DEFAULT_DIRECTORY
    #define PIVOT_SHADER_DEFAULT_DIRECTORY "."
#endif

#ifndef PIVOT_SHADER_CACHE_DEFAULT_DIRECTORY
    #define PIVOT_SHADER_CACHE_DEFAULT_DIRECTORY "./shaders_cache"
#endif

namespace pivot::graphics
{

class ShaderStorage
{
public:
    ShaderStorage();
    ShaderStorage(const ShaderStorage &) = delete;
    ~ShaderStorage();

    void setOptions(const VulkanShader::VulkanVersion &ver,
                    const VulkanShader::OptimizationLevel &op = VulkanShader::OptimizationLevel::None);
    void recompile(const std::string &name);
    void recompile();
    std::string load(const std::filesystem::path &path, const bool bForceCompile = false);

    const VulkanShader &get(const std::string &shaderName) const { return shaderStorage.at(shaderName); }

private:
    std::optional<VulkanShader> getCacheBinary(const std::filesystem::path &path, const bool bForceCompile);
    VulkanShader compileAndCache(const std::filesystem::path &path);

public:
    std::filesystem::path shader_path = PIVOT_SHADER_DEFAULT_DIRECTORY;
    std::filesystem::path cache_path = PIVOT_SHADER_CACHE_DEFAULT_DIRECTORY;

private:
    VulkanShader::OptimizationLevel optimizationLevel = VulkanShader::OptimizationLevel::None;
    VulkanShader::VulkanVersion vulkanVersion;
    std::unordered_map<std::string, VulkanShader> shaderStorage;
};

}    // namespace pivot::graphics
