#include <catch2/catch_test_macros.hpp>

#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/Component/ref.hxx>

using namespace pivot::ecs::data;
using namespace pivot::ecs::component;

TEST_CASE("Component refs works", "[component]")
{
    auto description = GlobalIndex::getSingleton().getDescription("Tag").value();
    auto array = description.createContainer(description);

    ComponentRef ref(*array, 0);
    REQUIRE_THROWS(ref.get());

    Value original_tag{Record{{"name", "bob"}}};
    array->setValueForEntity(0, original_tag);
    REQUIRE(ref.get() == original_tag);
    // PLD DoD
    REQUIRE(ref.get() == array->getValueForEntity(0));
    // PLD DoD
    REQUIRE(static_cast<Value>(ref) == original_tag);

    Value new_tag{Record{{"name", "alice"}}};
    ref.set(new_tag);
    // PLD DoD
    REQUIRE(array->getValueForEntity(0) == new_tag);

    // PLD DoD
    ref = original_tag;
    REQUIRE(array->getValueForEntity(0) == original_tag);

    REQUIRE(ref.description() == description);
}
