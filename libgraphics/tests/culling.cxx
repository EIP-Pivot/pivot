#include <catch2/catch.hpp>
#include <pivot/graphics/culling.hxx>
#include <pivot/graphics/types/vk_types.hxx>

TEST_CASE("culling works", "[culling]")
{
    // Raw data of a camera with the following properties:
    // position: 0,0,0
    // front: 0,0,-1
    // up: 0,1,0
    pivot::graphics::gpuObject::CameraData camera{
        .position = glm::vec4(1.0f),
        .viewproj = glm::mat4(0.756078, 0.000000, -0.000000, -0.000000, 0.000000, -1.428148, 0.000000, 0.000000,
                              -0.000000, 0.000000, -1.001001, -1.000000, 0.000000, 0.000000, -0.100100, 0.000000),
    };
    Transform transform_1{{0, 0, -1}, {0, 0, 0}, {1, 1, 1}};
    Transform transform_2{{0, 0, 1}, {0, 0, 0}, {1, 1, 1}};
    MeshBoundingBox box{{-1, -1, -1}, {1, 1, 1}};

    REQUIRE(pivot::graphics::culling::should_object_be_rendered(transform_1, box, camera));
    REQUIRE(!pivot::graphics::culling::should_object_be_rendered(transform_2, box, camera));
}
