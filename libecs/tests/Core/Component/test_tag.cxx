#include <catch2/catch.hpp>
#include <pivot/ecs/Components/Tag.hxx>
#include <pivot/ecs/Core/Component/description_helpers.hxx>
#include <pivot/ecs/Core/Component/index.hxx>

using namespace pivot::ecs::component;
using namespace pivot::ecs::data;
using namespace pivot::ecs::component::helpers;

TEST_CASE("Correct description for tag component", "[component][tag]")
{
    REQUIRE(GlobalIndex::getSingleton().getComponentNameByType<Tag>() == "Tag");
    auto description = GlobalIndex::getSingleton().getDescription("Tag").value();

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
