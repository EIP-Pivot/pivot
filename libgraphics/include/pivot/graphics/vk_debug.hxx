#pragma once

#include <string_view>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/vk_utils.hxx"

/// Debug utils for vulkan
namespace pivot::graphics::vk_debug
{
template <vk_utils::wrappedVulkanType T>
/// Set the debug name of the object
void setObjectName(vk::Device &device, const T &object, const std::string_view &name)
{
#ifndef NDEBUG
    vk::DebugUtilsObjectNameInfoEXT nameInfo{
        .objectType = object.objectType,
        .objectHandle = (uint64_t)(typename T::CType)object,
        .pObjectName = name.data(),
    };
    device.setDebugUtilsObjectNameEXT(nameInfo);
#endif
}

template <vk_utils::wrappedVulkanType T, typename D>
/// Set the debug tag of the object
void setObjectTag(vk::Device &device, const T &object, const std::span<D> &tag)
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
void beginRegion(vk::CommandBuffer &cmdbuffer, const char *pMarkerName, const std::array<float, 4> color);
/// End a debug region
void endRegion(vk::CommandBuffer cmdBuffer);

};    // namespace pivot::graphics::vk_debug
