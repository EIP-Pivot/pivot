#include <iostream>

#include <pivot/graphics/culling.hxx>

namespace pivot::graphics
{

bool culling::should_object_be_rendered(const Transform &transform, const MeshBoundingBox &box,
                                        const gpuObject::CameraData &camera)
{
    auto projection = camera.viewproj * transform.getModelMatrix();
    bool result = false;
    for (const auto &point: box.vertices()) {
        const auto position = projection * glm::vec4(point.x, point.y, point.z, 1.0);
        result = result || (position.x <= position.w && position.x >= -position.w && position.y <= position.w &&
                            position.y >= -position.w && position.z <= position.w && position.z >= 0);
    }
    return result;
}
}    // namespace pivot::graphics::culling
