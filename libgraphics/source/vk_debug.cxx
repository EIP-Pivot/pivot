#include "pivot/graphics/vk_debug.hxx"

namespace pivot::graphics::vk_debug
{

void beginRegion(vk::CommandBuffer &cmdbuffer, const char *pMarkerName, const std::array<float, 4> color)
{
    vk::DebugUtilsLabelEXT markerInfo{
        .pLabelName = pMarkerName,
        .color = color,
    };
    cmdbuffer.beginDebugUtilsLabelEXT(markerInfo);
};

void endRegion(vk::CommandBuffer cmdBuffer) { cmdBuffer.endDebugUtilsLabelEXT(); }

};    // namespace pivot::graphics::vk_debug