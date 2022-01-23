#ifndef PIVOT_ENGINE_GRAPHICS_CULLING_HXX
#define PIVOT_ENGINE_GRAPHICS_CULLING_HXX

#include <pivot/graphics/types/UniformBufferObject.hxx>
#include <pivot/graphics/types/vk_types.hxx>

namespace pivot::graphics::culling
{
bool should_object_be_rendered(const Transform &transform, const MeshBoundingBox &box, const CameraData &camera);
}

#endif    // PIVOT_ENGINE_GRAPHICS_CULLING_HXX
