#include <catch2/catch_test_macros.hpp>
#include <sstream>

#include <pivot/script/Builtins.hxx>

using namespace pivot::ecs;
using namespace pivot::ecs::data;
using namespace pivot::ecs::script::interpreter::builtins;

std::string print(const std::vector<data::Value> &params)
{
    std::ostringstream oss;
    pivot::ecs::script::interpreter::builtins::builtin_print_stream(params, oss);
    return oss.str();
}

TEST_CASE("Builtin print", "[script][builtin]")
{
    REQUIRE(print({3}) == "3\n");
    REQUIRE(print({22, 0.7}) == "22 0.7\n");
    REQUIRE(print({"lol"}) == "lol\n");
    REQUIRE(print({true}) == "true\n");
    REQUIRE(print({data::Asset{"cube"}}) == "Asset(cube)\n");
    REQUIRE(print({glm::vec3(1, 2, 3)}) == "vec3(1,2,3)\n");
}

bool equal(const data::Value &left, const data::Value &right)
{
    return std::get<bool>(builtin_operator<Operator::Equal>(left, right));
}

TEST_CASE("Builtin equal", "[script][builtin]")
{
    REQUIRE(equal(Value{3}, Value{3}));
    REQUIRE(equal(Value{"test"}, Value{"test"}));
    REQUIRE_FALSE(equal(Value{3}, Value{4}));
    REQUIRE_THROWS(equal(Value{"haha"}, Value{2}));
}
