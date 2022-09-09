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

TEST_CASE("Builtin equal", "[script][builtin]")
{
    auto equal = generic_builtin_comparator(std::equal_to<void>{}, "==");

    REQUIRE(equal(Value{3}, Value{3}));
    REQUIRE(equal(Value{"test"}, Value{"test"}));
    REQUIRE_FALSE(equal(Value{3}, Value{4}));
    REQUIRE_THROWS(equal(Value{"haha"}, Value{2}));
}

TEST_CASE("Builtin not equal", "[script][builtin]")
{
    auto equal = generic_builtin_comparator(std::not_equal_to<void>{}, "!");

    REQUIRE_FALSE(equal(Value{3}, Value{3}));
    REQUIRE_FALSE(equal(Value{"test"}, Value{"test"}));
    REQUIRE(equal(Value{3}, Value{4}));
    REQUIRE_THROWS(equal(Value{"haha"}, Value{2}));
}

TEST_CASE("Builtin vec3", "[script][builtin]")
{
    auto context = BuiltinContext::mock();

    auto result = builtin_vec3({{1.}, {2.}, {3.}}, context);
    REQUIRE(std::holds_alternative<glm::vec3>(result));
    REQUIRE(std::get<glm::vec3>(result) == glm::vec3{1, 2, 3});
}
