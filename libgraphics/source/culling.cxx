#include <iostream>

#include <pivot/graphics/culling.hxx>

namespace pivot::graphics::culling
{

bool should_object_be_rendered(const RenderObject &object, const MeshBoundingBox &box,
                               const ICamera::GPUCameraData &camera)
{
    bool result = false;
    for (const auto &point: box.vertices()) {
        const auto position = camera.viewproj * object.objectInformation.transform.getModelMatrix() *
                              glm::vec4(point.x, point.y, point.z, 1.0);
        result = result || (position.x <= position.w && position.x >= -position.w && position.y <= position.w &&
                            position.y >= -position.w && position.z <= position.w && position.z >= 0);
    }
    return result;
}
}    // namespace pivot::graphics::culling
