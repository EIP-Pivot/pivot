#pragma once

#include "pivot/graphics/VulkanShader.hxx"

#include <filesystem>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace pivot::graphics
{

class ShaderStorage
{
public:
    static const std::filesystem::path cache_path;

public:
    ShaderStorage();
    ~ShaderStorage();

    void setOptions(const VulkanShader::VulkanVersion &ver,
                    const VulkanShader::OptimizationLevel &op = VulkanShader::OptimizationLevel::None);
    void recompile(const std::string &name);
    void recompile();
    void load(const std::filesystem::path &path, const bool bForceCompile = false);

    const VulkanShader &get(const std::string &shaderName) const { return shaderStorage.at(shaderName); }

private:
    std::optional<VulkanShader> getCacheBinary(const std::filesystem::path &path, const bool bForceCompile);
    VulkanShader compileAndCache(const std::filesystem::path &path);

private:
    VulkanShader::OptimizationLevel optimizationLevel = VulkanShader::OptimizationLevel::None;
    VulkanShader::VulkanVersion vulkanVersion;
    std::unordered_map<std::string, VulkanShader> shaderStorage;
};

}    // namespace pivot::graphics
