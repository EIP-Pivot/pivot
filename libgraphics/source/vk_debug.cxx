#include "pivot/graphics/vk_debug.hxx"

namespace pivot::graphics::vk_debug
{

void beginRegion(vk::CommandBuffer &cmdbuffer, const char *pMarkerName, const std::array<float, 4> color)
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

void endRegion(vk::CommandBuffer &cmdBuffer)
{
#ifndef NDEBUG
    cmdBuffer.endDebugUtilsLabelEXT();
#endif
}

};    // namespace pivot::graphics::vk_debug
