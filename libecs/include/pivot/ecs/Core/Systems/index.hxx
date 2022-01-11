#pragma once

#include "pivot/ecs/Core/Component/manager.hxx"
#include "pivot/ecs/Core/EntityManager.hxx"
#include "pivot/ecs/Core/Systems/description.hxx"

#include <atomic>
#include <map>
#include <mutex>
#include <optional>
#include <typeindex>


namespace pivot::ecs::systems
{
class Index
{
public:
    template <typename F>
    void registerSystem(const Description &description, F &&system)
    {
        description.validate();
        if (m_descriptionByName.contains(description.name)) { throw DuplicateError(description.name); }

        m_descriptionByName.insert({description.name, description});
        m_systemsByName.insert(
            {description.name,
             [system, description](component::Manager &componentManager, EntityManager &entityManager) {
                 std::vector<component::Manager::ComponentId> componentsId;
                 componentsId.reserve(description.arguments.size());

                 for (const auto &component: description.arguments) {
                     componentsId.push_back(componentManager.GetComponentId(component).value());
                 }
                 for (const auto &entity: entityManager.getEntities()) {
                     for (const auto &componentId: componentsId) {
                         std::cout << componentManager.GetComponent(entity.first, componentId).value().type().name()
                                   << std::endl;    // DenseComponentArray -> std::any en T : Type de cast inconnue dans
                                                    // la fonction
                     }
                 }
             }});    // make wrapper with system
    }

    std::optional<Description> getDescription(const std::string &systemName) const;

    std::optional<std::function<void(component::Manager &, EntityManager &)>>
    getSystemByName(const std::string &systemName);

    std::optional<std::function<void(component::Manager &, EntityManager &)>>
    getSystemByDescription(const Description &description);

    using const_iterator = std::map<std::string, Description>::const_iterator;
    const_iterator begin() const;
    const_iterator end() const;

    std::vector<std::string> getAllSystemsNames() const;

    struct DuplicateError : public std::logic_error {
        DuplicateError(const std::string &systemName);

        std::string systemName;
    };

private:
    std::map<std::string, Description> m_descriptionByName;
    std::map<std::string, std::function<void(component::Manager &, EntityManager &)>> m_systemsByName;
};

class GlobalIndex : private Index
{
public:
    template <typename F>
    void registerSystem(const Description &description, F &&system)
    {
        if (m_read_only) { throw std::logic_error("Cannot modify global system index after program started"); }

        const std::lock_guard<std::mutex> guard(m_mutex);

        this->Index::registerSystem(description, system);
    }

    std::optional<Description> getDescription(const std::string &componentName);

    std::optional<std::function<void(component::Manager &, EntityManager &)>>
    getSystemByName(const std::string &systemName);

    std::optional<std::function<void(component::Manager &, EntityManager &)>>
    getSystemByDescription(const Description &description);
    
    Index::const_iterator begin();
    Index::const_iterator end();
    std::vector<std::string> getAllSystemsNames();

    static GlobalIndex &getSingleton();

private:
    std::atomic<bool> m_read_only;
    std::mutex m_mutex;

    void lockReadOnly();
};

}    // namespace pivot::ecs::systems