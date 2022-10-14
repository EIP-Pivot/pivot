#pragma once

#include <string>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/vk_utils.hxx"
#include "pivot/pivot.hxx"

/// Debug utils for vulkan
namespace pivot::graphics::vk_debug
{
template <vk_utils::wrappedVulkanType T>
/// Set the debug name of the object
FORCEINLINE void setObjectName([[maybe_unused]] vk::Device &device, [[maybe_unused]] const T &object,
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
FORCEINLINE void setObjectTag([[maybe_unused]] vk::Device &device, [[maybe_unused]] const T &object,
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
FORCEINLINE void beginRegion(vk::CommandBuffer &cmdbuffer, const char *pMarkerName, const std::array<float, 4> color)
{
    pivotAssertMsg(pMarkerName, "Command region's name is NULL");
#ifndef NDEBUG
    vk::DebugUtilsLabelEXT markerInfo{
        .pLabelName = pMarkerName,
        .color = color,
    };
    cmdbuffer.beginDebugUtilsLabelEXT(markerInfo);
#endif
};

/// End a debug region
FORCEINLINE void endRegion(vk::CommandBuffer &cmdBuffer)
{
#ifndef NDEBUG
    cmdBuffer.endDebugUtilsLabelEXT();
#endif
}

};    // namespace pivot::graphics::vk_debug
