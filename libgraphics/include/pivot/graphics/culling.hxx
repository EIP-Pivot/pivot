#pragma once

#include <pivot/graphics/types/UniformBufferObject.hxx>
#include <pivot/graphics/types/vk_types.hxx>

namespace pivot::graphics::culling
{
bool should_object_be_rendered(const Transform &transform, const gpu_object::MeshBoundingBox &box,
                               const CameraData &camera);
}
