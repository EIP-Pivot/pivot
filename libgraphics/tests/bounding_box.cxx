#include <pivot/graphics/types/AABB.hxx>

#include <catch2/catch_test_macros.hpp>
#include <iostream>

using namespace pivot::graphics::gpu_object;
using namespace pivot::graphics;

TEST_CASE("bounding box calculation works", "[bounding_box]")
{
    AABB box({0, 0, 0});

    box.addPoint({1, -1, 0});
    REQUIRE(box.low == glm::vec3(0, -1, 0));
    REQUIRE(box.high == glm::vec3(1, 0, 0));
}

TEST_CASE("bounding box constructor is correct", "[bounding_box]")
{
    std::vector<Vertex> vertex = {
        Vertex{
            .pos = {0, 0, 0},
        },
        Vertex{
            .pos = {1, -1, 0},
        },
    };
    AABB box(vertex);
    REQUIRE(box.low == glm::vec3(0, -1, 0));
    REQUIRE(box.high == glm::vec3(1, 0, 0));
}

TEST_CASE("bounding box iteration", "[bounding_box]")
{
    AABB box({0, 0, 0}, {1, 1, 1});
    std::vector<glm::vec3> expectedVertices{
        {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1},
    };
    auto vertices = box.vertices();

    REQUIRE(vertices.size() == expectedVertices.size());
    REQUIRE(std::equal(vertices.begin(), vertices.end(), expectedVertices.begin()));
}
