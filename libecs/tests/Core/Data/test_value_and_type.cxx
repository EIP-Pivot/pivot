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
    REQUIRE(Value{Void{}}.type() == Type{BasicType::Void});

    REQUIRE(Value{Record{{"position", Value{glm::vec3{1, 2, 3}}}, {"speed", Value{420.69}}}}.type() ==
            Type{RecordType{{{"speed", Type{BasicType::Number}}, {"position", Type{BasicType::Vec3}}}}});

    Type t{RecordType{{"bool", BasicType::Boolean},
                      {"int", BasicType::Integer},
                      {"num", BasicType::Number},
                      {"str", BasicType::String},
                      {"vec", BasicType::Vec3},
                      {"void", BasicType::Void}}};
    Value v{Record{{"bool", Value{false}},
                   {"int", Value{0}},
                   {"num", Value{0.0}},
                   {"str", Value{""}},
                   {"vec", Value{glm::vec3{0, 0, 0}}},
                   {"void", Value{Void{}}}}};
    REQUIRE(t.defaultValue() == v);
}

TEST_CASE("Check that a type is a subset of another", "[ecs][data]")
{
    REQUIRE(Type{BasicType::Integer}.isSubsetOf(Type{BasicType::Integer}));
    REQUIRE_FALSE(Type{BasicType::Integer}.isSubsetOf(Type{BasicType::Asset}));
    REQUIRE_FALSE(Type{BasicType::Integer}.isSubsetOf(Type{RecordType{}}));
    REQUIRE(Type{RecordType{}}.isSubsetOf(Type{RecordType{}}));
    REQUIRE_FALSE(Type{RecordType{{"test", Type{BasicType::Integer}}}}.isSubsetOf(Type{RecordType{}}));
    REQUIRE(Type{RecordType{{"test", Type{BasicType::Integer}}}}.isSubsetOf(
        Type{RecordType{{"test", Type{BasicType::Integer}}}}));
    REQUIRE_FALSE(Type{RecordType{{"test", Type{BasicType::Integer}}}}.isSubsetOf(
        Type{RecordType{{"test", Type{BasicType::Asset}}}}));
    REQUIRE(Type{RecordType{{"test", Type{BasicType::Integer}}}}.isSubsetOf(
        Type{RecordType{{"test", Type{BasicType::Integer}}, {"other", Type{BasicType::Asset}}}}));
}
