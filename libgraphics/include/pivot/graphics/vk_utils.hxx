#pragma once

#include <Logger.hpp>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/PivotException.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"

namespace pivot::graphics::vk_utils
{

template <typename T>
concept wrappedVulkanType = requires
{
    typename T::CType;
    requires requires(T a) { a.objectType; };
};

/// Test if a vk::Result is considered as a success.
constexpr void vk_try(vk::Result err)
{
    if (err < vk::Result::eSuccess) throw std::runtime_error(vk::to_string(err));
}

/// @copydoc vk_try
constexpr void vk_try(VkResult res) { vk_try(vk::Result(res)); }

/// Read a whole file into a vector of byte.
template <typename T = std::byte>
requires std::is_trivial_v<T> && std::is_standard_layout_v<T> std::vector<T>
readBinaryFile(const std::filesystem::path &filename)
{
    assert(!std::filesystem::is_symlink(filename));
    size_t fileSize = std::filesystem::file_size(filename);
    std::ifstream file(filename, std::ios::binary);
    std::vector<T> fileContent(fileSize / sizeof(T));

    if (!file.is_open()) throw std::runtime_error("failed to open file " + filename.string());
    file.read(reinterpret_cast<char *>(fileContent.data()), fileSize * sizeof(T));
    assert(!file.bad());
    assert(!file.fail());
    assert(fileContent.size() == (unsigned long)file.gcount());
    file.close();
    return fileContent;
}

/// Read a whole file into a string
std::string readFile(const std::filesystem::path &filename);

template <typename T = std::byte>
std::size_t writeBinaryFile(const std::filesystem::path &filename, std::span<const T> &code)
{
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<const char *>(code.data()), code.size_bytes());
    file.close();
    return code.size();
}

/// Create a vk::ShaderModule from bytecode.
vk::ShaderModule createShaderModule(const vk::Device &device, std::span<const std::byte> code);

/// Return the max MSAA sample supported by the device
vk::SampleCountFlagBits getMaxUsableSampleCount(vk::PhysicalDevice &physical_device);
/// Return the optimal format supported by the device from a given set of candidates
vk::Format findSupportedFormat(vk::PhysicalDevice &gpu, const std::vector<vk::Format> &candidates,
                               vk::ImageTiling tiling, vk::FormatFeatureFlags features);

namespace tools
{
    /// Print KB/MB/GB value from a number of bytes.
    std::string bytesToString(uint64_t bytes);
}    // namespace tools

}    // namespace pivot::graphics::vk_utils
