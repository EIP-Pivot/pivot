#include <catch2/catch_test_macros.hpp>

#include <pivot/ecs/Core/Data/value.hxx>

using namespace pivot::ecs::data;

TEST_CASE("Check that values and types work", "[data]")
{
    REQUIRE(Value{"hello"}.type() == Type{BasicType::String});
    REQUIRE(Value{3.0}.type() == Type{BasicType::Number});
    REQUIRE(Value{3}.type() == Type{BasicType::Integer});
    REQUIRE(Value{bool{true}}.type() == Type{BasicType::Boolean});
    REQUIRE(Value{glm::vec3{1, 2, 3}}.type() == Type{BasicType::Vec3});

    REQUIRE(Value{Record{{"position", Value{glm::vec3{1, 2, 3}}}, {"speed", Value{420.69}}}}.type() ==
            Type{RecordType{{{"speed", Type{BasicType::Number}}, {"position", Type{BasicType::Vec3}}}}});
}
