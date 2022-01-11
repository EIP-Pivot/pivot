#pragma once

#include "pivot/ecs/Core/Component/manager.hxx"
#include "pivot/ecs/Core/EntityManager.hxx"
#include "pivot/ecs/Core/Systems/description.hxx"
#include "pivot/ecs/Core/Systems/function_traits.hxx"
#include "pivot/ecs/Core/Systems/index.hxx"
#include <tuple>

#include <functional>
#include <unordered_map>

namespace pivot::ecs::systems
{

class Manager
{
public:
    bool useSystem(const Description &description)
    {
        std::optional<std::function<void(component::Manager &, EntityManager &)>> system =
            GlobalIndex::getSingleton().getSystemByDescription(description);
        if (!system.has_value()) return false;

        m_systems.at(description.name) = GlobalIndex::getSingleton().getSystemByDescription(description).value();
        return true;
    }

    template <typename F>
    // requires std::is_invocable_v<F, Args...> // Args pi être, pi être pas
    void registerSystem(const Description &description, F &&systems)
    {
        // Déménage dans le SystemIndex
        // m_systems.insert({description.name,
        //                   [systems, description](component::Manager &componentManager, EntityManager &entityManager)
        //                   {
        //                       std::vector<component::Manager::ComponentId> componentsId;
        //                       componentsId.reserve(description.arguments.size());

        //                       for (const auto &component: description.arguments) {
        //                           componentsId.push_back(componentManager.GetComponentId(component).value());
        //                       }
        //                       for (const auto &entity: entityManager.getEntities()) {
        //                           for (const auto &componentId: componentsId) {
        //                               std::cout << componentManager.GetComponent(entity.first,
        //                               componentId).value().type().name() << std::endl; // DenseComponentArray ->
        //                               std::any en T : Type de cast inconnue dans la fonction
        //                           }
        //                       }
        //                   }});
    }

    void execute(component::Manager &componentManager, EntityManager &entityManager)
    {
        for (const auto &[name, system]: m_systems) {
            std::cout << "Executing: " + name << std::endl;
            system(componentManager, entityManager);
        }
    }

private:
    // std::unordered_map<std::string, std::function<void(component::Manager &, EntityManager &)>> m_systems;
    std::unordered_map<std::string, std::function<void(component::Manager &, EntityManager &)>> m_systems;
};

}    // namespace pivot::ecs::systems