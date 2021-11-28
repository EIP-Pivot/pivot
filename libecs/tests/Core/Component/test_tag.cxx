#include <catch2/catch.hpp>
#include <pivot/ecs/Components/Tag.hxx>

using namespace pivot::ecs::component;

TEST_CASE("Correct description for tag component", "[component][tag]")
{
    auto &description = Tag::description;

    REQUIRE(description.name == "Tag");
    REQUIRE(description.properties.size() == 1);
    REQUIRE(description.properties[0].name == "name");
    REQUIRE(description.properties[0].type == Description::Property::Type::STRING);

    const auto original_tag_name = "a name";
    auto tag = description.create({{"name", original_tag_name}});
    REQUIRE(tag.has_value());
    REQUIRE(tag.type() == typeid(Tag));
    REQUIRE(std::any_cast<Tag>(tag).name == original_tag_name);
    auto tag_name = description.getProperty(tag, "name");
    REQUIRE(std::get_if<std::string>(&tag_name) != nullptr);
    REQUIRE(std::get<std::string>(tag_name) == original_tag_name);

    const auto new_tag_name = "a new name";
    description.setProperty(tag, "name", new_tag_name);
    tag_name = description.getProperty(tag, "name");
    REQUIRE(std::get<std::string>(tag_name) == new_tag_name);

    REQUIRE_THROWS(description.create({}));
    REQUIRE_THROWS(description.create({{"name", "something"}, {"unknown_property", "value"}}));
    REQUIRE_THROWS(description.getProperty(tag, "unknown"));
    REQUIRE_THROWS(description.setProperty(tag, "unknown", ""));
    REQUIRE_THROWS(description.setProperty(tag, "name", 3));

    auto not_a_tag = std::make_any<int>(42);
    REQUIRE_THROWS(description.getProperty(not_a_tag, "name"));
    REQUIRE_THROWS(description.setProperty(not_a_tag, "name", ""));
}
