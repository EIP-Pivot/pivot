#include "pivot/ecs/Components/Gravity.hxx"
#include "pivot/ecs/Components/RigidBody.hxx"
#include "pivot/ecs/Components/Tag.hxx"
#include "pivot/ecs/Core/EntityManager.hxx"
#include "pivot/ecs/Core/Systems/manager.hxx"
#include <catch2/catch.hpp>
#include <pivot/ecs/Core/Scene.hxx>
#include <pivot/ecs/Core/Systems/description.hxx>

using namespace pivot::ecs::systems;

void test_manager_registration(std::vector<std::any> components) {
    std::cout << "I'm a system with components:\n";
    for(const auto &component: components)
        std::cout << "\t" << component.type().name() << "\n";
    auto tag = std::any_cast<std::reference_wrapper<Tag>>(components[1]);
    std::cout << tag.get().name << std::endl;
    tag.get().name = "oui";
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
    cManager.AddComponent(entity, Tag{ .name = "non"}, tagId);
    cManager.AddComponent(entity, RigidBody{}, rigidId);
    cManager.AddComponent(entity, Gravity{}, gravId);
    
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
