#include <catch2/catch.hpp>
#include <pivot/ecs/Core/Component/array.hxx>
#include <pivot/ecs/Core/Component/index.hxx>

using namespace pivot::ecs::component;

namespace
{
std::unique_ptr<IComponentArray> emptyCreateContainer(Description) { return std::unique_ptr<IComponentArray>(nullptr); }

Description emptyComponent(const std::string &name) { return {name, {}, emptyCreateContainer}; }
}    // namespace

class TestType1
{
};
class TestType2
{
};

TEST_CASE("correct synchronisation", "[component][index]")
{
    GlobalIndex index;

    index.registerComponent(emptyComponent("Test Component"));
    REQUIRE_THROWS_AS(index.registerComponent(emptyComponent("Test Component")), Index::DuplicateError);
    index.registerComponent(emptyComponent("Test Component 2"));

    REQUIRE(index.getDescription("Unknown") == std::nullopt);
    REQUIRE_THROWS(index.registerComponent(emptyComponent("Unknown")));
    REQUIRE(index.getDescription("Test Component").value().name == "Test Component");
}
