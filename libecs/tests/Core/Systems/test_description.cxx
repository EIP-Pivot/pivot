#include "pivot/ecs/Components/Gravity.hxx"
#include "pivot/ecs/Components/RigidBody.hxx"
#include "pivot/ecs/Components/Tag.hxx"
#include <pivot/ecs/Core/Systems/description.hxx>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include <iostream>

using namespace pivot::ecs;

std::vector<event::Event> test_description(const systems::Description &, component::ArrayCombination &,
                                           const event::EventWithComponent &)
{
    return {};
}

const event::Description testEvent{
    .name = "Colid",
    .entities = {"entity"},
    .payload = pivot::ecs::data::BasicType::Number,
};

TEST_CASE("valid system description", "[description]")
{
    systems::Description description{
        .name = "Test Description",
        .systemComponents =
            {
                "RigidBody",
                "Tag",
            },
        .eventListener = testEvent,
        .eventComponents = {{"Tag"}},
        .system = &test_description,
    };
    REQUIRE_NOTHROW(description.validate());
}

TEST_CASE("Empty system description", "[description]")
{
    systems::Description description{
        .name = "Test Description",
        .systemComponents = {},
        .eventListener = testEvent,
        .eventComponents = {{"Tag"}},
        .system = &test_description,
    };
    REQUIRE_NOTHROW(description.validate());
}

TEST_CASE("Empty args system description", "[description]")
{
    systems::Description description{
        .name = "Invalid",
        .systemComponents = {},
        .eventListener = testEvent,
        .eventComponents = {},
        .system = &test_description,
    };
    REQUIRE_THROWS_WITH(description.validate(), "Event require 1 entities given 0");
}
