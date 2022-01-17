#include "pivot/ecs/Core/Systems/manager.hxx"

namespace pivot::ecs::systems
{

bool Manager::useSystem(const Description &description)
{
    if (m_systems.contains(description.name))
        throw EcsException("System already use.");

    std::optional<std::function<void(component::Manager &, EntityManager &)>> system =
        GlobalIndex::getSingleton().getSystemByDescription(description);

    if (!system.has_value()) return false;

    m_systems.insert({description.name, GlobalIndex::getSingleton().getSystemByDescription(description).value()});
    return true;
}

bool Manager::useSystem(const std::string &systemName)
{
    if (m_systems.contains(systemName)) throw EcsException("System already use.");

    std::optional<Description> description = GlobalIndex::getSingleton().getDescription(systemName);

    if (!description.has_value())
        return false;

    std::optional<std::function<void(component::Manager &, EntityManager &)>> system =
        GlobalIndex::getSingleton().getSystemByDescription(description.value());

    if (!system.has_value()) return false;

    m_systems.insert({systemName, GlobalIndex::getSingleton().getSystemByDescription(description.value()).value()});
    return true;
}

void Manager::execute(component::Manager &componentManager, EntityManager &entityManager)
{
    for (const auto &[name, system]: m_systems) {
        std::cout << "Executing: " + name << std::endl;
        system(componentManager, entityManager);
    }
}

std::vector<std::string> Manager::getSystemUsed()
{
    std::vector<std::string> systems;
    for (const auto &[name, system]: m_systems) {
        systems.push_back(name);
    }
    return systems;
}

}    // namespace pivot::ecs::systems