#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>

#include <pivot/ecs/Core/Data/value.hxx>
#include <pivot/ecs/Core/Data/value_serialization.hxx>

using namespace nlohmann;
using namespace pivot::ecs::data;

TEST_CASE("Values can be serialized", "[data][save]")
{
    REQUIRE(json(Value{"hello"}).dump() == "\"hello\"");
    REQUIRE(json(Value{3.0}).dump() == "3.0");
    REQUIRE(json(Value{3}).dump() == "3");
    REQUIRE(json(Value{true}).dump() == "true");
    REQUIRE(json(Value{glm::vec3{1, 2, 3}}).dump() == "[1.0,2.0,3.0]");
    REQUIRE(json(Value{Record{{"name", "bob"}, {"age", 42}}}).dump() == R"({"age":42,"name":"bob"})");
}

TEST_CASE("Values can be deserialized", "[data][save]")
{
    REQUIRE(json::parse("\"hello\"").get<Value>() == Value{"hello"});
    REQUIRE(json::parse("3.0").get<Value>() == Value{3.0});
    REQUIRE(json::parse("3").get<Value>() == Value{3});
    REQUIRE(json::parse("false").get<Value>() == Value{false});
    REQUIRE(json::parse("[1.0,2.0,3.0]").get<Value>() == Value{glm::vec3{1, 2, 3}});
    REQUIRE(json::parse(R"({"age":42,"name":"bob"})").get<Value>() == Value{Record{{"name", "bob"}, {"age", 42}}});

    REQUIRE_THROWS(json::parse("[1.0,2.0,3.0,4.0]").get<Value>());
    REQUIRE_THROWS(json::parse("[1.0,2.0]").get<Value>());
    REQUIRE_THROWS(json::parse("[1.0,2.0,null]").get<Value>());
    REQUIRE_THROWS(json::parse(R"({"age":42,"name":null})").get<Value>());
    REQUIRE_THROWS(json::parse("null").get<Value>());
}
