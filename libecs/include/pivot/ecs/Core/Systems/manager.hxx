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
/** \brief Manages all the systems in a Scene
 *
 * System are execut when a event is emit
 */
class Manager
{
public:
    /// Manager constructor: take component manager and entity manager to build event object and find entity component
    Manager(component::Manager &componentManager, EntityManager &entityManager)
        : m_componentManager(componentManager), m_entityManager(entityManager){};

    /// register system in a scene by his name
    void useSystem(const std::string &systemName);
    /// register system in a scene by his description
    void useSystem(const Description &description);
    /// execute the system listening the event
    std::vector<event::Event> execute(const event::Event &event);

    /// Constant iterator over every system used
    using const_iterator = std::map<std::string, Description>::const_iterator;
    const_iterator begin() const;    ///< Begin iterator
    const_iterator end() const;      ///< End iterator

    /// Returns true if the system is registered in the manager
    bool hasSystem(const std::string &systemName) const { return m_systems.contains(systemName); }

    /// Error thrown when component are not registered
    struct MissingComponent : public std::runtime_error {
        /// Created a MissingComponent based on a component's name
        MissingComponent(const std::string &componentName);
        /// Name of the missing component
        std::string componentName;
    };
    /// Error thrown when system is not registered
    struct MissingSystem : public std::runtime_error {
        /// Created a MissingSystem based on a system's name
        MissingSystem(const std::string &systemName);
        /// Name of the missing system
        std::string systemName;
    };

private:
    std::vector<component::Manager::ComponentId> getComponentsId(const std::vector<std::string> &components);
    std::vector<event::Event> executeOne(const Description &, const event::Event &);
    component::Manager &m_componentManager;
    EntityManager &m_entityManager;
    std::map<std::string, Description> m_systems;
    std::unordered_map<std::string, component::ArrayCombination> m_combinations;
};

}    // namespace pivot::ecs::systems
