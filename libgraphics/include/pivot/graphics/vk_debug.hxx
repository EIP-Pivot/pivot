#pragma once

#include <string>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/vk_utils.hxx"

/// Debug utils for vulkan
namespace pivot::graphics::vk_debug
{
template <vk_utils::wrappedVulkanType T>
/// Set the debug name of the object
void setObjectName([[maybe_unused]] vk::Device &device, [[maybe_unused]] const T &object,
                   [[maybe_unused]] const std::string &name)
{
#ifndef NDEBUG
    vk::DebugUtilsObjectNameInfoEXT nameInfo{
        .objectType = object.objectType,
        .objectHandle = (uint64_t)(typename T::CType)object,
        .pObjectName = name.c_str(),
    };
    device.setDebugUtilsObjectNameEXT(nameInfo);
#endif
}

template <vk_utils::wrappedVulkanType T, typename D>
/// Set the debug tag of the object
void setObjectTag([[maybe_unused]] vk::Device &device, [[maybe_unused]] const T &object,
                  [[maybe_unused]] const std::span<D> &tag)
{
#ifndef NDEBUG
    vk::DebugUtilsObjectTagInfoEXT tagInfo{
        .objectType = object.objectType,
        .objectHandle = (uint64_t)(typename T::CType)object,
        .tagSize = tag.size_bytes(),
        .pTag = tag.data(),
    };
    device.setDebugUtilsObjectTagEXT(tagInfo);
#endif
}

/// Begin a new debug region
void beginRegion([[maybe_unused]] vk::CommandBuffer &cmdbuffer, [[maybe_unused]] const char *pMarkerName,
                 [[maybe_unused]] const std::array<float, 4> color);
/// End a debug region
void endRegion([[maybe_unused]] vk::CommandBuffer &cmdBuffer);

};    // namespace pivot::graphics::vk_debug
