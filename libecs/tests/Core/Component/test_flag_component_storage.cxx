#include <catch2/catch_test_macros.hpp>

#include <pivot/ecs/Components/Tag.hxx>
#include <pivot/ecs/Core/Component/FlagComponentStorage.hxx>
#include <pivot/ecs/Core/Component/index.hxx>

using namespace pivot::ecs;
using namespace pivot::ecs::component;
using namespace pivot::ecs::data;

TEST_CASE("Flag component storage", "[component][scene]")
{
    FlagComponentStorage storage(Tag::description);

    REQUIRE(storage.getDescription() == Tag::description);
    REQUIRE(storage.maxEntity() == 0);
    REQUIRE_FALSE(storage.entityHasValue(0));
    REQUIRE_FALSE(storage.entityHasValue(4));
    REQUIRE_FALSE(storage.getValueForEntity(0).has_value());

    REQUIRE_NOTHROW(storage.setValueForEntity(3, Void{}));
    REQUIRE(storage.maxEntity() == 3);
    REQUIRE(storage.getValueForEntity(3) == Value{Void{}});

    REQUIRE_NOTHROW(storage.setValueForEntity(3, std::nullopt));
    REQUIRE(storage.maxEntity() == 3);
    REQUIRE_FALSE(storage.getValueForEntity(3).has_value());

    REQUIRE_THROWS(storage.setValueForEntity(2, Value{3}));
}
