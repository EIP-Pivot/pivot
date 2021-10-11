#ifndef PIVOT_ENGINE_GRAPHICS_CULLING_HXX
#define PIVOT_ENGINE_GRAPHICS_CULLING_HXX

#include <pivot/graphics/Camera.hxx>
#include <pivot/graphics/types/RenderObject.hxx>

namespace pivot::graphics::culling
{
bool should_object_be_rendered(const RenderObject &object, const MeshBoundingBox &box,
                               const ICamera::GPUCameraData &camera);
}

#endif    // PIVOT_ENGINE_GRAPHICS_CULLING_HXX
