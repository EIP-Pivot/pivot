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

void Manager::execute(const event::Description &eventDescription, const data::Value &payload, const std::vector<Entity> &entities)
{
    for (const auto &[name, description]: m_systems) {
        if (eventDescription.name == description.eventListener.name) {

            Description::systemArgs componentArrays;
            for (const auto index: getComponentsId(description.systemComponents))
                componentArrays.push_back(m_componentManager->GetComponentArray(index).value());
            std::cout << description.eventComponents.size() << std::endl;
            if (entities.size() != description.eventComponents.size())
                throw std::logic_error("This system expect " + std::to_string(description.eventComponents.size()) + " entity.");
            std::vector<std::vector<data::Value>> entitiesComponents;
            for (std::size_t i = 0; i < entities.size(); i++) {
                std::vector<data::Value> entityComponents;
                for (const auto index: getComponentsId(description.eventComponents[i]))
                    entityComponents.push_back(m_componentManager->GetComponent(entities[i], index).value());
                entitiesComponents.push_back(entityComponents);
            }
            event::Event event{
                .description = eventDescription,
                .entities = entitiesComponents,
                .payload = payload,
            };
            description.system(description, componentArrays, event);
        }
    }
}

std::vector<std::string> Manager::getSystemUsed()
{
    std::vector<std::string> systems;
    for (const auto &[name, system]: m_systems) { systems.push_back(name); }
    return systems;
}

std::vector<component::Manager::ComponentId> Manager::getComponentsId(const std::vector<std::string> &components)
{
    std::vector<component::Manager::ComponentId> componentsId;

    for (const auto &component: components)
        componentsId.push_back(m_componentManager->GetComponentId(component).value());
    return componentsId;
}

}    // namespace pivot::ecs::systems