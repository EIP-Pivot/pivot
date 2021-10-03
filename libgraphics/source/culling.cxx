#include <iostream>

#include <pivot/graphics/culling.hxx>

namespace culling
{
bool should_object_be_rendered(const RenderObject &object, const ICamera::GPUCameraData &camera)
{
    auto transform = gpuObject::Transform(object.objectInformation.transform);
    auto modelMatrix = transform.translation * transform.rotation * transform.scale;
    auto position = camera.viewproj * modelMatrix * glm::vec4(0.0, 0.0, 0.0, 1.0);
    return position.x <= position.w && position.x >= -position.w && position.y <= position.w &&
           position.y >= -position.w && position.z <= position.w && position.z >= 0;
}
}    // namespace culling
