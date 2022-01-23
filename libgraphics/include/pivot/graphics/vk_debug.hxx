#pragma once

#include <glm/vec4.hpp>
#include <vulkan/vulkan.hpp>

/// Debug utils for vulkan
namespace pivot::graphics::vk_debug
{
template <class T>
/// Set the debug name of the object
void setObjectName(vk::Device &device, const T &object, const std::string &name)
{
    vk::DebugUtilsObjectNameInfoEXT nameInfo{
        .objectType = object.objectType,
        .objectHandle = (uint64_t)(typename T::CType)object,
        .pObjectName = name.c_str(),
    };
    device.setDebugUtilsObjectNameEXT(nameInfo);
}
/// Begin a new debug region
void beginRegion(vk::CommandBuffer &cmdbuffer, const char *pMarkerName, const std::array<float, 4> color);
/// End a debug region
void endRegion(vk::CommandBuffer cmdBuffer);

};    // namespace pivot::graphics::vk_debug