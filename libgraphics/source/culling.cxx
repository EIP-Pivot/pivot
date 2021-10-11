#include <iostream>

#include <pivot/graphics/culling.hxx>

namespace culling
{

bool should_object_be_rendered(const RenderObject &object, const MeshBoundingBox &box,
                               const ICamera::GPUCameraData &camera)
{

    auto transform = gpuObject::Transform(object.objectInformation.transform);
    auto modelMatrix = transform.translation * transform.rotation * transform.scale;
    bool result = false;
    for (auto &point: box.vertices()) {
        auto position = camera.viewproj * modelMatrix * glm::vec4(point.x, point.y, point.z, 1.0);
        result = result || position.x <= position.w && position.x >= -position.w && position.y <= position.w &&
                               position.y >= -position.w && position.z <= position.w && position.z >= 0;
    }
    return result;
}
}    // namespace culling
