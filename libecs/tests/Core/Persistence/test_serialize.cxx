#include <catch2/catch.hpp>
#include <stdexcept>
#include <iostream>

#include "pivot/ecs/Components/Tag.hxx"
#include "pivot/ecs/Core/Component/description.hxx"
#include "pivot/ecs/Core/Persistence/Component.hxx"

using namespace pivot::ecs::component;
using namespace pivot::ecs::persistence;

Description::Property::ValueType getProperty(std::any component, std::string property)
{
    return Description::Property::ValueType("oui");
}

void setProperty(std::any component, std::string property, Description::Property::ValueType value)
{
    throw std::logic_error("setProperty not implemented");
}

std::any create(std::map<std::string, Description::Property::ValueType> properties)
{
    throw std::logic_error("Create not implemented");
}

TEST_CASE("serialize component", "[peristence][component]")
{
    std::any component = std::make_any<int>(3);
    Description tagDescription{.name = "tag",
                               .properties = {{.name = "name", .type = Description::Property::Type::STRING}},
                               .getProperty = getProperty,
                               .setProperty = setProperty,
                               .create = create};

    std::string json = serializeComponent(tagDescription, component);
    REQUIRE(json == R"({"name":"tag","properties":{"name":"oui"}})");
}
