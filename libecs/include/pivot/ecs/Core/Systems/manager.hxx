#pragma once

#include "pivot/ecs/Core/Component/manager.hxx"
#include "pivot/ecs/Core/EcsException.hxx"
#include "pivot/ecs/Core/EntityManager.hxx"
#include "pivot/ecs/Core/Systems/description.hxx"
#include "pivot/ecs/Core/Systems/index.hxx"

#include <functional>
#include <tuple>
#include <unordered_map>

namespace pivot::ecs::systems
{

class Manager
{
public:
    Manager(component::Manager &componentManager, EntityManager &entityManager)
        : m_componentManager(componentManager), m_entityManager(entityManager){};

    void useSystem(const std::string &systemName);
    void useSystem(const Description &description);

    void execute(const event::Event &event);

    using const_iterator = std::unordered_map<std::string, Description>::const_iterator;
    const_iterator begin() const;
    const_iterator end() const;

    struct MissingComponent : public std::runtime_error {
        MissingComponent(const std::string &componentName);

        std::string componentName;
    };

    struct MissingSystem : public std::runtime_error {
        MissingSystem(const std::string &systemName);

        std::string systemName;
    };

private:
    std::vector<component::Manager::ComponentId> getComponentsId(const std::vector<std::string> &components);
    void executeOne(const Description &, const event::Event &);
    component::Manager &m_componentManager;
    EntityManager &m_entityManager;
    std::unordered_map<std::string, Description> m_systems;
    std::unordered_map<std::string, Description::systemArgs> m_combinations;
};

}    // namespace pivot::ecs::systems