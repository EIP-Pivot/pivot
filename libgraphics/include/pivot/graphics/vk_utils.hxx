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
std::vector<T> readBinaryFile(const std::filesystem::path &filename)
{
    size_t fileSize = 0;
    std::vector<T> fileContent;
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) { throw std::runtime_error("failed to open file " + filename.string()); }
    fileSize = file.tellg();
    fileContent.resize(fileSize);
    file.seekg(0);
    file.read((char *)fileContent.data(), fileSize);
    file.close();
    return fileContent;
}

/// Read a whole file into a string
std::string readFile(const std::filesystem::path &filename);

template <typename T = std::byte>
std::size_t writeBinaryFile(const std::filesystem::path &filename, const std::vector<T> &code)
{
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<const char *>(code.data()), code.size() * sizeof(T));
    file.close();
    return code.size();
}

/// Create a vk::ShaderModule from bytecode.
vk::ShaderModule createShaderModule(const vk::Device &device, std::span<const std::uint32_t> code);

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
