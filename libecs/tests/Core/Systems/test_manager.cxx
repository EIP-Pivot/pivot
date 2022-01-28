#include "pivot/ecs/Components/Gravity.hxx"
#include "pivot/ecs/Components/RigidBody.hxx"
#include "pivot/ecs/Components/Tag.hxx"
#include "pivot/ecs/Core/EntityManager.hxx"
#include "pivot/ecs/Core/Systems/manager.hxx"
#include <catch2/catch.hpp>
#include <pivot/ecs/Core/Scene.hxx>
#include <pivot/ecs/Core/Systems/description.hxx>
#include <pivot/ecs/Core/Data/value.hxx>

using namespace pivot::ecs;
using namespace pivot::ecs::data;

void test_manager_registration(const systems::Description::availableEntities &e, const systems::Description &description,
                               const systems::Description::systemArgs &entities, const event::Event &event)
{
    std::cout << "I'm a system:\n";
    auto &tagArray = entities[1].get();
    tagArray.getValueForEntity(0).has_value();
    auto tag = tagArray.getValueForEntity(0).value();
    std::cout << std::get<std::string>(std::get<Record>(tag).at("name")) << std::endl;
    std::get<std::string>(std::get<Record>(tag).at("name")) = "edit";
    tagArray.setValueForEntity(0, tag);
}

TEST_CASE("Manager register system", "[description][registration][manager]")
{
    component::Manager cManager;
    component::Description tag =
        component::GlobalIndex::getSingleton().getDescription("Tag").value();
    component::Description rigid =
        component::GlobalIndex::getSingleton().getDescription("RigidBody").value();
    component::Description grav =
        component::GlobalIndex::getSingleton().getDescription("Gravity").value();

    component::Manager::ComponentId tagId = cManager.RegisterComponent(tag);
    component::Manager::ComponentId rigidId = cManager.RegisterComponent(rigid);
    component::Manager::ComponentId gravId = cManager.RegisterComponent(grav);

    EntityManager eManager;
    Entity entity = eManager.CreateEntity();
    cManager.AddComponent(entity, Value{Record{{"name", "oui"}}}, tagId);
    cManager.AddComponent(entity, Value{Record{{"velocity", glm::vec3(0.0f)}, {"acceleration", glm::vec3(0.0f)}}},
                          rigidId);
    cManager.AddComponent(entity, Value{Record{{"force", glm::vec3(0.0f)}}}, gravId);

    systems::Manager manager;
    systems::Description description{
        .name = "Manager",
        .arguments =
            {
                "RigidBody",
                "Tag",
            },
        .system = &test_manager_registration,
    };
    systems::GlobalIndex::getSingleton().registerSystem(description);
    manager.useSystem(description);
    manager.execute(cManager, eManager);
    manager.execute(cManager, eManager);
}
