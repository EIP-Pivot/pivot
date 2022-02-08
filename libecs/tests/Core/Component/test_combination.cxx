#include <catch2/catch.hpp>

#include <pivot/ecs/Core/Component/combination.hxx>
#include <pivot/ecs/Core/Component/index.hxx>

using namespace pivot::ecs::data;
using namespace pivot::ecs::component;

TEST_CASE("Component array combinations", "[component]")
{
    auto description = GlobalIndex::getSingleton().getDescription("Tag").value();
    auto array1 = description.createContainer(description);
    auto array2 = description.createContainer(description);

    Value name1{Record{{"name", "bobby"}}};
    Value name2{Record{{"name", "max"}}};

    array1->setValueForEntity(0, name1);
    array2->setValueForEntity(0, name2);
    array1->setValueForEntity(3, name1);
    array2->setValueForEntity(5, name2);
    array1->setValueForEntity(7, name1);
    array2->setValueForEntity(7, name2);

    ArrayCombination combinations{{std::ref(*array1), std::ref(*array2)}};

    unsigned count = 0;
    for (auto combination: combinations) {
        REQUIRE(combination[0].get() == name1);
        REQUIRE(combination[1].get() == name2);
        count++;
    }
    REQUIRE(count == 2);

    (*combinations.begin())[0].set(name2);
    REQUIRE(array1->getValueForEntity(0) == name2);
}
