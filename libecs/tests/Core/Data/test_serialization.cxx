#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <pivot/ecs/Core/Data/value.hxx>
#include <pivot/ecs/Core/Data/value_serialization.hxx>

using namespace nlohmann;
using namespace pivot::ecs::data;
using EntityRef = pivot::EntityRef;

TEST_CASE("Values can be serialized", "[data][save]")
{
    REQUIRE(json(Value{"hello"}).dump() == "\"hello\"");
    REQUIRE(json(Value{3.0}).dump() == "3.0");
    REQUIRE(json(Value{3}).dump() == "3");
    REQUIRE(json(Value{true}).dump() == "true");
    REQUIRE(json(Value{glm::vec3{1, 2, 3}}).dump() == "[1.0,2.0,3.0]");
    REQUIRE(json(Value{glm::vec2{1, 2}}).dump() == "[1.0,2.0]");
    REQUIRE(json(Value{Record{{"name", "bob"}, {"age", 42}}}).dump() == R"({"age":42,"name":"bob"})");
    REQUIRE(json(Value{Asset{"cube"}}).dump() == R"({"asset":{"name":"cube"}})");
    REQUIRE(json(Value{Color{5, 4, 3, 2}}).dump() == R"({"color":{"rgba":[5.0,4.0,3.0,2.0]}})");
    REQUIRE(json(Value{Void{}}).dump() == "null");
    REQUIRE(json(Value{EntityRef::empty()}).dump() == R"({"entity":null})");
    REQUIRE(json(Value{EntityRef{42}}).dump() == R"({"entity":42})");
}

TEST_CASE("Values can be deserialized", "[data][save]")
{
    REQUIRE(json::parse("\"hello\"").get<Value>() == Value{"hello"});
    REQUIRE(json::parse("3.0").get<Value>() == Value{3.0});
    REQUIRE(json::parse("3").get<Value>() == Value{3});
    REQUIRE(json::parse("false").get<Value>() == Value{false});
    REQUIRE(json::parse("[1.0,2.0,3.0]").get<Value>() == Value{glm::vec3{1, 2, 3}});
    REQUIRE(json::parse("[1.0,2.0]").get<Value>() == Value{glm::vec2{1, 2}});
    REQUIRE(json::parse(R"({"age":42,"name":"bob"})").get<Value>() == Value{Record{{"name", "bob"}, {"age", 42}}});
    REQUIRE(json::parse(R"({"asset":{"name":"cube"}})").get<Value>() == Value{Asset{"cube"}});
    REQUIRE(json::parse(R"({"color":{"rgba":[5.0,4.0,3.0,2.0]}})").get<Value>() == Value{Color{5, 4, 3, 2}});
    REQUIRE(json::parse("null").get<Value>() == Value{Void{}});
    REQUIRE(json::parse(R"({"entity":null})").get<Value>() == Value{EntityRef::empty()});
    REQUIRE(json::parse(R"({"entity":42})").get<Value>() == Value{EntityRef{42}});

    REQUIRE_THROWS(json::parse("[1.0,2.0,3.0,4.0]").get<Value>());
    REQUIRE_THROWS(json::parse("[1.0]").get<Value>());
    REQUIRE_THROWS(json::parse("[1.0,2.0,null]").get<Value>());
    REQUIRE_THROWS(json::parse(R"({"age":42,"name":[]})").get<Value>());
}
