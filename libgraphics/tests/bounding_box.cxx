#include <logging.hxx>
#include <pivot/graphics/types/Mesh.hxx>

#include <catch2/catch.hpp>

TEST_CASE("bounding box calculation works", "[bounding_box]")
{
    MeshBoundingBox box({0, 0, 0});

    box.addPoint({1, -1, 0});
    REQUIRE(box.low == glm::vec3(0, -1, 0));
    REQUIRE(box.high == glm::vec3(1, 0, 0));
}

TEST_CASE("bounding box iteration", "[bounding_box]")
{
    MeshBoundingBox box({0, 0, 0}, {1, 1, 1});
    std::vector<glm::vec3> expectedVertices{
        {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1},
    };
    auto vertices = box.vertices();

    REQUIRE(vertices.size() == expectedVertices.size());
    REQUIRE(std::equal(vertices.begin(), vertices.end(), expectedVertices.begin()));
}
