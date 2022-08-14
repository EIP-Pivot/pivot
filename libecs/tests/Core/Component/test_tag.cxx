#include <catch2/catch_test_macros.hpp>
#include <pivot/ecs/Components/Tag.hxx>
#include <pivot/ecs/Components/TagArray.hxx>
#include <pivot/ecs/Core/Component/description_helpers.hxx>
#include <pivot/ecs/Core/Component/index.hxx>

using namespace pivot::ecs;
using namespace pivot::ecs::component;
using namespace pivot::ecs::data;
using namespace pivot::ecs::component::helpers;

TEST_CASE("Correct description for tag component", "[component][tag]")
{
    auto description = Tag::description;

    REQUIRE(description.name == "Tag");
    REQUIRE(description.type == Type{RecordType{{"name", Type{BasicType::String}}}});

    const auto original_tag_name = "a name";
    Tag tag{original_tag_name};
    Value value = Helpers<Tag>::createValueFromType(tag);
    REQUIRE(value == Value{Record{{"name", original_tag_name}}});

    const auto new_tag_name = "a new name";
    Helpers<Tag>::updateTypeWithValue(tag, Value{Record{{"name", new_tag_name}}});
    value = Helpers<Tag>::createValueFromType(tag);
    REQUIRE(value == Value{Record{{"name", new_tag_name}}});

    REQUIRE_THROWS(Helpers<Tag>::updateTypeWithValue(tag, Value{3}));
    REQUIRE_THROWS(Helpers<Tag>::updateTypeWithValue(tag, Value{Record{}}));
}

TEST_CASE("Tag array works", "[component][ecs]")
{
    TagArray array(Tag::description);

    auto name1 = Value{Record{{"name", "name"}}};
    auto name2 = Value{Record{{"name", "other name"}}};

    REQUIRE_NOTHROW(array.setValueForEntity(0, name1));
    REQUIRE_NOTHROW(array.setValueForEntity(1, name2));
    REQUIRE_THROWS_AS(array.setValueForEntity(2, name1), TagArray::DuplicateEntityTag);
    REQUIRE_NOTHROW(array.setValueForEntity(0, std::nullopt));
    REQUIRE_NOTHROW(array.setValueForEntity(2, name1));

    REQUIRE(array.getEntityID("name") == 2);
    REQUIRE(array.getEntityID("other name") == 1);
    REQUIRE(array.getEntityID("no one") == std::nullopt);
}
