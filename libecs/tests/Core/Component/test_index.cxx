#include <catch2/catch.hpp>
#include <pivot/ecs/Core/Component/array.hxx>
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
std::unique_ptr<IComponentArray> emptyCreateContainer() { return std::unique_ptr<IComponentArray>(nullptr); }

Description emptyComponent(const std::string &name)
{
    return {name, {}, emptyGetProperty, emptySetProperty, emptyCreate, emptyCreateContainer};
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
    GlobalIndex index;

    index.registerComponent(emptyComponent("Test Component"));
    REQUIRE_THROWS_AS(index.registerComponent(emptyComponent("Test Component")), Index::DuplicateError);
    index.registerComponent(emptyComponent("Test Component 2"));

    index.registerComponentWithType<TestType1>(emptyComponent("TestType1"));
    REQUIRE_THROWS_AS(index.registerComponentWithType<TestType1>(emptyComponent("TestType1")), Index::DuplicateError);

    REQUIRE(index.getDescription("Unknown") == std::nullopt);
    REQUIRE(index.getComponentNameByType<TestType2>() == std::nullopt);
    REQUIRE_THROWS(index.registerComponent(emptyComponent("Unknown")));

    REQUIRE(index.getDescription("Test Component").value().name == "Test Component");
    REQUIRE(index.getComponentNameByType<TestType1>().value() == "TestType1");
}
