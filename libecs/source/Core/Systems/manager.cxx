#include "pivot/ecs/Core/Systems/manager.hxx"

namespace pivot::ecs::systems
{

bool Manager::useSystem(const std::string &systemName)
{
    std::optional<Description> description = GlobalIndex::getSingleton().getDescription(systemName);

    if (!description.has_value()) return false;

    return useSystem(description.value());
}

bool Manager::useSystem(const Description &description)
{
    if (m_systems.contains(description.name)) throw EcsException("System already use.");
    m_systems.insert({description.name, description});
    return true;
}

void Manager::execute(const event::Event &event)
{
    for (const auto &[name, description]: m_systems) {
        if (event.description.name == description.eventListener.name) {
            Description::systemArgs componentArrays;

            for (const auto &component: description.components) {
                auto index = m_componentManager->GetComponentId(component).value();
                componentArrays.push_back(m_componentManager->GetComponentArray(index).value());
            }

            Description::availableEntities a;
            for (const auto &[entity, _]: m_entityManager->getEntities()) {
                if (hasAllComponents(entity, componentArrays))
                    a.push_back(entity);
            }
            description.system(a, description, componentArrays, event);
        }
    }
}

std::vector<std::string> Manager::getSystemUsed()
{
    std::vector<std::string> systems;
    for (const auto &[name, system]: m_systems) { systems.push_back(name); }
    return systems;
}

bool Manager::hasAllComponents(Entity entity, const Description::systemArgs &componentArrays)
{
    for (const auto &componentArray: componentArrays) {
        auto &array = componentArray.get();
        if (!array.getValueForEntity(entity).has_value())
            return false;
    }
    return true;
}

}    // namespace pivot::ecs::systems