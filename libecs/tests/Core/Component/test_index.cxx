#include <catch2/catch.hpp>
#include <pivot/ecs/Core/Component/index.hxx>

using namespace pivot::ecs::component;

namespace
{
Description::Property::ValueType emptyGetProperty(std::any component, std::string property)
{
    return Description::Property::ValueType(0);
}
void emptySetProperty(std::any component, std::string property, Description::Property::ValueType value) {}
std::any emptyCreate(std::map<std::string, Description::Property::ValueType> properties) { return std::any(); }
Description emptyComponent(const std::string &name)
{
    return {name, {}, emptyGetProperty, emptySetProperty, emptyCreate};
}
}    // namespace

TEST_CASE("correct synchronisation", "[component][index]")
{
    GlobalIndex index;

    index.registerComponent(emptyComponent("Test Component"));
    REQUIRE_THROWS_AS(index.registerComponent(emptyComponent("Test Component")), Index::DuplicateError);
    index.registerComponent(emptyComponent("Test Component 2"));
    REQUIRE_THROWS(index.getDescription("Unknown"));
    REQUIRE_THROWS(index.registerComponent(emptyComponent("Unknown")));
    REQUIRE(index.getDescription("Test Component").name == "Test Component");
}
