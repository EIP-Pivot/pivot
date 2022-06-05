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

/// Store compiled shader and handle caching
class ShaderStorage
{
public:
    /// Ctor
    ShaderStorage();
    ShaderStorage(const ShaderStorage &) = delete;
    /// Dtor
    ~ShaderStorage();

    /// @brief Set compilation options
    ///
    /// Will recompile all the currently loaded shader
    void setOptions(const VulkanShader::VulkanVersion &ver,
                    const VulkanShader::OptimizationLevel &op = VulkanShader::OptimizationLevel::None);
    /// Recompile a specific shader
    void recompile(const std::string &name);
    /// Recompile all shader
    void recompile();
    /// @brief Load a shader
    ///
    /// If the shader is already available nothing will happend, and the id will be returned.
    /// If bForceCompile is true, the shader provided will be reloaded and recompile no matter what.
    ///
    /// Might throw if the shader can't be compiled
    std::string load(const std::filesystem::path &path, const bool bForceCompile = false);
    /// @copybrief load
    ///
    /// If the shader is already available nothing will happend, and the id will be returned.
    /// If bForceCompile is true, the shader provided will be reloaded and recompile no matter what.
    ///
    /// If the shader can't be compiled, will return std::nullopt;
    std::optional<std::string> try_load(const std::filesystem::path &path, const bool bForceCompile = false);

    /// Return the actual shader object
    const VulkanShader &get(const std::string &shaderName) const { return shaderStorage.at(shaderName); }

private:
    std::optional<VulkanShader> getCacheBinary(const std::filesystem::path &path, const bool bForceCompile);
    VulkanShader compileAndCache(const std::filesystem::path &path);

public:
    /// Base path for the shader
    std::filesystem::path shader_path = PIVOT_SHADER_DEFAULT_DIRECTORY;
    /// Base path for the shader cache
    std::filesystem::path cache_path = PIVOT_SHADER_CACHE_DEFAULT_DIRECTORY;

private:
#ifndef NDEBUG
    VulkanShader::OptimizationLevel optimizationLevel = VulkanShader::OptimizationLevel::Performance;
#else
    VulkanShader::OptimizationLevel optimizationLevel = VulkanShader::OptimizationLevel::None;
#endif
    VulkanShader::VulkanVersion vulkanVersion;
    std::unordered_map<std::string, VulkanShader> shaderStorage;
};

}    // namespace pivot::graphics
