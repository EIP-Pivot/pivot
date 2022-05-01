#pragma once

#include <Logger.hpp>
#include <cstddef>
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
std::vector<std::byte> readFile(const std::string &filename);
/// Create a vk::ShaderModule from bytecode.
vk::ShaderModule createShaderModule(const vk::Device &device, const std::vector<std::byte> &code);

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
