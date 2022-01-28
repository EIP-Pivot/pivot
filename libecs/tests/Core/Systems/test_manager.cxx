#include "pivot/ecs/Components/Gravity.hxx"
#include "pivot/ecs/Components/RigidBody.hxx"
#include "pivot/ecs/Components/Tag.hxx"
#include "pivot/ecs/Core/EntityManager.hxx"
#include "pivot/ecs/Core/Systems/manager.hxx"
#include <catch2/catch.hpp>
#include <pivot/ecs/Core/Scene.hxx>
#include <pivot/ecs/Core/Systems/description.hxx>
#include <pivot/ecs/Core/Data/value.hxx>

using namespace pivot::ecs::systems;
using namespace pivot::ecs::data;

void test_manager_registration(Description::systemArgs &entities)
{
    std::cout << "I'm a system with components:\n";
    for (auto &entity: entities) {
        for (const auto &component: entity) {
            std::cout << "\t" << component.second.type().name() << "\n";
        }

        auto &[description, component] = entity[1];
        auto tag = std::any_cast<Value>(component);
        std::cout << std::get<std::string>( std::get<Record>(tag).at("name") ) << std::endl;
        std::get<std::string>( std::get<Record>(tag).at("name") ) = "non";
        std::cout << std::get<std::string>(std::get<Record>(tag).at("name")) << std::endl;
    }
}

TEST_CASE("Manager register system", "[description][registration][manager]")
{
    pivot::ecs::component::Manager cManager;
    pivot::ecs::component::Description tag =
        pivot::ecs::component::GlobalIndex::getSingleton().getDescription("Tag").value();
    pivot::ecs::component::Description rigid =
        pivot::ecs::component::GlobalIndex::getSingleton().getDescription("RigidBody").value();
    pivot::ecs::component::Description grav =
        pivot::ecs::component::GlobalIndex::getSingleton().getDescription("Gravity").value();

    pivot::ecs::component::Manager::ComponentId tagId = cManager.RegisterComponent(tag);
    pivot::ecs::component::Manager::ComponentId rigidId = cManager.RegisterComponent(rigid);
    pivot::ecs::component::Manager::ComponentId gravId = cManager.RegisterComponent(grav);

    EntityManager eManager;
    Entity entity = eManager.CreateEntity();
    cManager.AddComponent(entity, Value{Record{{"name", "oui"}}}, tagId);
    cManager.AddComponent(entity, Value{Record{{"velocity", glm::vec3(0.0f)}, {"acceleration", glm::vec3(0.0f)}}},
                          rigidId);
    cManager.AddComponent(entity, Value{Record{{"force", glm::vec3(0.0f)}}}, gravId);

    Manager manager;
    Description description {
        .name = "Manager",
        .arguments =
            {
                "RigidBody",
                "Tag",
            },
        .system = &test_manager_registration,
    };
    GlobalIndex::getSingleton().registerSystem(description);
    manager.useSystem(description);
    manager.execute(cManager, eManager);
    manager.execute(cManager, eManager);
}
