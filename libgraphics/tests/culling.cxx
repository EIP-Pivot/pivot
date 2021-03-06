#include <catch2/catch_test_macros.hpp>
#include <pivot/graphics/culling.hxx>
#include <pivot/graphics/types/vk_types.hxx>

using namespace pivot::graphics;
using namespace pivot::graphics::gpu_object;

TEST_CASE("culling works", "[culling]")
{
    // Raw data of a camera with the following properties:
    // position: 0,0,0
    // front: 0,0,-1
    // up: 0,1,0
    CameraData camera{
        .position = glm::vec4(1.0f),
        .view = glm::mat4(1.0f),
        .projection = glm::mat4(1.0f),
        .viewProjection = glm::mat4(0.756078, 0.000000, -0.000000, -0.000000, 0.000000, -1.428148, 0.000000, 0.000000,
                                    -0.000000, 0.000000, -1.001001, -1.000000, 0.000000, 0.000000, -0.100100, 0.000000),
    };
    Transform transform_1{{0, 0, -1}, {0, 0, 0}, {1, 1, 1}};
    Transform transform_2{{0, 0, 1}, {0, 0, 0}, {1, 1, 1}};
    Transform transform_big{{0, 0, -1}, {0, 0, 0}, {10, 10, 10}};
    AABB box{{-1, -1, -1}, {1, 1, 1}};

    REQUIRE(culling::should_object_be_rendered(transform_1, box, camera));
    REQUIRE(!culling::should_object_be_rendered(transform_2, box, camera));
    REQUIRE(culling::should_object_be_rendered(transform_big, box, camera));
}
