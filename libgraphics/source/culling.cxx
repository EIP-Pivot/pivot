#include <iostream>

#include <pivot/graphics/culling.hxx>

namespace pivot::graphics::culling
{

constexpr static std::array<glm::vec4, 6> PLANES = {{
    {-1, 0, 0, 1},
    {1, 0, 0, 1},
    {0, -1, 0, 1},
    {0, 1, 0, 1},
    {0, 0, -1, 1},
    {0, 0, 1, 0},
}};

bool should_object_be_rendered(const Transform &transform, const MeshBoundingBox &box,
                               const gpuObject::CameraData &camera)
{
    auto projection = camera.viewproj * transform.getModelMatrix();
    auto bounding_box = box.vertices();
    std::array<glm::vec4, bounding_box.size()> projected_points;
    for (unsigned i = 0; i < bounding_box.size(); i++) {
        projected_points[i] = projection * glm::vec4(bounding_box[i].x, bounding_box[i].y, bounding_box[i].z, 1.0);
    }
    for (auto plane: PLANES) {
        if (!std::any_of(projected_points.begin(), projected_points.end(),
                         [=](auto point) { return glm::dot(point, plane) >= 0; })) {
            return false;
        }
    }
    return true;
}
}    // namespace pivot::graphics::culling
