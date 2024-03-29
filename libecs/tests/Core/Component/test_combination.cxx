#include <catch2/catch_test_macros.hpp>

#include <pivot/ecs/Components/Tag.hxx>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/ScriptingComponentArray.hxx>
#include <pivot/ecs/Core/Component/combination.hxx>
#include <pivot/ecs/Core/Component/index.hxx>

using namespace pivot::ecs;
using namespace pivot::ecs::data;
using namespace pivot::ecs::component;

TEST_CASE("Component array combinations", "[component]")
{
    auto description = Tag::description;
    auto array1 = DenseTypedComponentArray<Tag>(description);
    auto array2 = DenseTypedComponentArray<Tag>(description);

    Value name1{Record{{"name", "bobby"}}};
    Value name2{Record{{"name", "max"}}};

    array1.setValueForEntity(0, name1);
    array2.setValueForEntity(0, name2);
    array1.setValueForEntity(3, name1);
    array2.setValueForEntity(5, name2);
    array1.setValueForEntity(7, name1);
    array2.setValueForEntity(7, name2);

    ArrayCombination combinations{{std::ref(array1), std::ref(array2)}};

    unsigned count = 0;
    for (auto combination: combinations) {
        REQUIRE(combination[0].get() == name1);
        REQUIRE(combination[1].get() == name2);
        count++;
    }
    REQUIRE(count == 2);

    (*combinations.begin())[0].set(name2);
    REQUIRE(array1.getValueForEntity(0) == name2);
}

std::unique_ptr<IComponentArray> createArray(Description d) { return std::make_unique<ScriptingComponentArray>(d); }

static const std::array<Description, 3> components{
    {{"1", BasicType::Integer, pivot::ecs::Provenance::builtin(), data::Value{0}, createArray},
     {"2", BasicType::Integer, pivot::ecs::Provenance::builtin(), data::Value{0}, createArray},
     {"3", BasicType::Integer, pivot::ecs::Provenance::builtin(), data::Value{0}, createArray}}};

static const std::vector<std::array<std::optional<Value>, components.size()>> entities{
    {std::nullopt, std::nullopt, std::nullopt},
    {1, 2, 3},    // Entity 1 has all components
    {1, 2, std::nullopt},
    {1, 2, 3},    // Entity 3 has all components
    {std::nullopt, std::nullopt, 3},
    {std::nullopt, std::nullopt, std::nullopt},

};

TEST_CASE("Array intersections work", "[component]")
{
    std::array<std::unique_ptr<IComponentArray>, components.size()> arrays;

    for (unsigned i = 0; i < 3; i++) { arrays[i] = components[i].createContainer(components[i]); }

    for (Entity entity = 0; entity < entities.size(); entity++) {
        for (unsigned i = 0; i < components.size(); i++) { arrays[i]->setValueForEntity(entity, entities[entity][i]); }
    }

    ArrayCombination combination{{*arrays[0], *arrays[1], *arrays[2]}};

    auto it = combination.begin();

    // PLD DoD
    REQUIRE(it != combination.end());
    REQUIRE(it->entity == 1);

    it++;

    REQUIRE(it != combination.end());
    REQUIRE(it->entity == 3);

    auto allComponents = it->getAllComponents();
    REQUIRE(allComponents.size() == 3);

    REQUIRE(allComponents[0].description() == components[0]);
    REQUIRE(allComponents[1].description() == components[1]);
    REQUIRE(allComponents[2].description() == components[2]);

    REQUIRE(allComponents[0].entity() == 3);
    REQUIRE(allComponents[1].entity() == 3);
    REQUIRE(allComponents[2].entity() == 3);

    it++;

    REQUIRE(it == combination.end());
}
