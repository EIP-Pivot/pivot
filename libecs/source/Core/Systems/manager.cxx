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

void Manager::execute(component::Manager &componentManager, EntityManager &entityManager)
{
    for (const auto &[name, system]: m_systems) {
        std::cout << "Executing: " + name << std::endl;
        system(componentManager, entityManager);
    }
}

}    // namespace pivot::ecs::systems