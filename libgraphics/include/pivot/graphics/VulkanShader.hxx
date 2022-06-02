#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <shaderc/shaderc.hpp>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/PivotException.hxx"

namespace pivot::graphics
{

/// @brief Represent a vulkan shader.
class VulkanShader
{
public:
    /// Error type for VulkanShader
    RUNTIME_ERROR(VulkanShader);

    /// Which optiomization level to use when compiling
    enum class OptimizationLevel {
        /// No optimization, enable debug symbols
        None = shaderc_optimization_level_zero,
        /// Optimize for size, no debug symbols
        Size = shaderc_optimization_level_size,
        /// Optimize for Performance, no debug symbols
        Performance = shaderc_optimization_level_performance,
    };

    /// What is the vulkan version targeted
    enum class VulkanVersion {
        e1_0 = shaderc_env_version_vulkan_1_0,
        e1_1 = shaderc_env_version_vulkan_1_1,
        e1_2 = shaderc_env_version_vulkan_1_2,
        e1_3 = shaderc_env_version_vulkan_1_3,
    };

public:
    /// Ctor, load @arg path from disk
    VulkanShader(const std::filesystem::path &path);
    /// Dtor
    ~VulkanShader();

    /// Return the name of the shader (the stem of the file)
    const std::string &getName() const noexcept { return name; }
    /// @brief Return the byte code of the shader.
    ///
    /// If the shader was not compiled before, it return an empty vector
    /// @see isCompiled
    constexpr const std::vector<std::uint32_t> &getByteCode() const noexcept { return byte_code; }
    /// @brief Return true is the shader has already be compiled
    ///
    /// Does not take into account any change in the source code that wasn't recompiled.
    constexpr bool isCompiled() const noexcept { return !byte_code.empty(); }
    /// @brief Hash the source code
    std::size_t getHash() const { return std::hash<std::string>{}(source_code); }

    /// @brief Set a macro to be set during the compilation
    void setMacro(const std::string &key, const std::string &value = "") { macros[key] = value; }

    /// Reload the source file from the disk
    void reload();
    /// @brief run only the preprocessor step.
    ///
    /// Return a string with the preprocess step.
    /// Similar to -E
    std::string pre_process(shaderc::CompileOptions options = {});

    /// @brief compile the code into byte code
    void compile(VulkanVersion version, OptimizationLevel level);

    /// @see isCompiled
    constexpr operator bool() const noexcept { return isCompiled(); }

private:
    shaderc::CompileOptions getCompileOptions(VulkanVersion version, OptimizationLevel level);

private:
    std::string name;
    std::filesystem::path shaderPath;
    std::string source_code;
    std::vector<std::uint32_t> byte_code;
    std::unordered_map<std::string, std::string> macros;
};

}    // namespace pivot::graphics

namespace std
{
template <>
/// Overload of the hash operator for the VulkanShader class
struct hash<pivot::graphics::VulkanShader> {
    /// Overload of the hash operator for the VulkanShader class
    std::size_t operator()(const pivot::graphics::VulkanShader &shader) const { return shader.getHash(); };
};
}    // namespace std
