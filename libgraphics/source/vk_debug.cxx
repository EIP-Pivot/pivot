#include "pivot/graphics/vk_debug.hxx"

namespace pivot::graphics::vk_debug
{

void beginRegion(vk::CommandBuffer &cmdbuffer, const char *pMarkerName, const std::array<float, 4> color)
{
    assert(pMarkerName);
#ifndef NDEBUG
    vk::DebugUtilsLabelEXT markerInfo{
        .pLabelName = pMarkerName,
        .color = color,
    };
    cmdbuffer.beginDebugUtilsLabelEXT(markerInfo);
#else
    (void)cmdbuffer;
    (void)pMarkerName;
    (void)color;
#endif
};

void endRegion(vk::CommandBuffer &cmdBuffer)
{
#ifndef NDEBUG
    cmdBuffer.endDebugUtilsLabelEXT();
#else
    (void)cmdBuffer;
#endif
}

};    // namespace pivot::graphics::vk_debug
