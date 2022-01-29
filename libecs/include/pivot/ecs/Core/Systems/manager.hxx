#pragma once

#include "pivot/ecs/Core/Component/manager.hxx"
#include "pivot/ecs/Core/EcsException.hxx"
#include "pivot/ecs/Core/EntityManager.hxx"
#include "pivot/ecs/Core/Systems/description.hxx"
#include "pivot/ecs/Core/Systems/index.hxx"

#include <tuple>
#include <functional>
#include <unordered_map>

namespace pivot::ecs::systems
{

class Manager
{
public:
    Manager(std::unique_ptr<component::Manager> &componentManager, std::unique_ptr<EntityManager> &entityManager)
        : m_componentManager(componentManager), m_entityManager(entityManager) {};
    
    bool useSystem(const std::string &systemName);
    bool useSystem(const Description &description);

    void execute(const event::Event &event);

    std::vector<std::string> getSystemUsed();
private:
    bool hasAllComponents(Entity entity, const Description::systemArgs &componentArrays);
    std::unique_ptr<component::Manager> &m_componentManager;
    std::unique_ptr<EntityManager> &m_entityManager;
    std::unordered_map<std::string, Description> m_systems;
};

}    // namespace pivot::ecs::systems