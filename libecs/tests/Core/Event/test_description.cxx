#include <pivot/ecs/Core/Event/description.hxx>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

using namespace pivot::ecs::event;

TEST_CASE("valid event description", "[description]")
{
    Description description{
        .name = "Colid",
        .entities = {"Entity 1", "Entity 2"},
        .payload = pivot::ecs::data::BasicType::Number,
    };
    REQUIRE_NOTHROW(description.validate());
}

TEST_CASE("Empty event description", "[description]")
{
    Description description;
    REQUIRE_THROWS_WITH(description.validate(), "Empty event name");
}
