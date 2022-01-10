#include <catch2/catch.hpp>
#include <pivot/ecs/Core/Systems/description.hxx>
#include "pivot/ecs/Components/Tag.hxx"
#include "pivot/ecs/Components/Gravity.hxx"
#include "pivot/ecs/Components/RigidBody.hxx"


#include <iostream>

using namespace pivot::ecs::systems;

void coucou(RigidBody, Tag, Gravity) {}

TEST_CASE("system description", "[systems]")
{
    const std::string test = "Control";
    Description description = Description::build_system_description(test, &coucou);
    std::cout << "Function name: " << description.name << std::endl;
    for (auto arg: description.arguments)
    {
        std::cout << "Function args: " << *arg << std::endl;
    }
}