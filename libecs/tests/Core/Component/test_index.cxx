#include <catch2/catch_test_macros.hpp>
#include <pivot/ecs/Core/Component/array.hxx>
#include <pivot/ecs/Core/Component/index.hxx>

using namespace pivot::ecs::component;

namespace
{
std::unique_ptr<IComponentArray> emptyCreateContainer(Description) { return std::unique_ptr<IComponentArray>(nullptr); }

Description emptyComponent(const std::string &name)
{
    return {name,
            {pivot::ecs::data::BasicType::Integer},
            pivot::ecs::Provenance::builtin(),
            pivot::ecs::data::Value{0},
            emptyCreateContainer};
}
}    // namespace

class TestType1
{
};
class TestType2
{
};

TEST_CASE("correct synchronisation", "[component][index]")
{
    Index index;

    index.registerComponent(emptyComponent("Test Component"));
    REQUIRE_THROWS_AS(index.registerComponent(emptyComponent("Test Component")), Index::DuplicateError);
    index.registerComponent(emptyComponent("Test Component 2"));

    REQUIRE(index.getDescription("Unknown") == std::nullopt);
    REQUIRE(index.getDescription("Test Component").value().name == "Test Component");
}
