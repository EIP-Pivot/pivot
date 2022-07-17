#include <catch2/catch_test_macros.hpp>
#include <pivot/builtins/systems/CollisionSystem.hxx>

using namespace pivot::builtins::systems::details;

const std::array entities = {
    EntityAABB{
        .low = {1, 2, 0},
        .high = {2, 4, 0},
        .entity = 1,
    },
    EntityAABB{
        .low = {0, -3, 0},
        .high = {3, 3, 0},
        .entity = 2,
    },
    EntityAABB{
        .low = {-2, -1, 0},
        .high = {2, 1, 0},
        .entity = 3,
    },
    EntityAABB{
        .low = {-3, -2, 0},
        .high = {4, 0, 0},
        .entity = 4,
    },
};

const std::vector<std::pair<Entity, Entity>> expectedCollisions = {{1, 2}, {2, 3}, {2, 4}, {3, 4}};

TEST_CASE("Collision detection", "[system][engine]")
{
    auto collisions = getEntityCollisions(entities);

    REQUIRE(collisions == expectedCollisions);
}
