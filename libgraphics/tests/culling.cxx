#include "Camera.hxx"
#include <catch2/catch.hpp>
#include <pivot/graphics/culling.hxx>
#include <pivot/graphics/types/vk_types.hxx>

TEST_CASE("culling works", "[culling]")
{
    Camera camera;
    Transform transform_1{{0, 0, -1}, {0, 0, 0}, {1, 1, 1}};
    Transform transform_2{{0, 0, 1}, {0, 0, 0}, {1, 1, 1}};
    MeshBoundingBox box{{-1, -1, -1}, {1, 1, 1}};

    REQUIRE(pivot::graphics::culling::should_object_be_rendered(transform_1, box, camera.getGPUCameraData()));
    REQUIRE(!pivot::graphics::culling::should_object_be_rendered(transform_2, box, camera.getGPUCameraData()));
}
