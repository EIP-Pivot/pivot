#include "pivot/ecs/Core/Systems/manager.hxx"

namespace pivot::ecs::systems
{
Manager::MissingComponent::MissingComponent(const std::string &componentName)
    : std::runtime_error("Missing component: " + componentName), componentName(componentName)
{
}

Manager::MissingSystem::MissingSystem(const std::string &systemName)
    : std::runtime_error("Missing component: " + systemName), systemName(systemName)
{
}

void Manager::useSystem(const std::string &systemName)
{
    std::optional<Description> description = GlobalIndex::getSingleton().getDescription(systemName);

    if (!description.has_value()) throw MissingSystem(systemName);

    useSystem(description.value());
}

void Manager::useSystem(const Description &description)
{
    if (m_systems.contains(description.name)) throw EcsException("System already use.");
    m_systems.insert({description.name, description});

    std::vector<std::reference_wrapper<component::IComponentArray>> componentArrays;
    for (const auto index: getComponentsId(description.systemComponents))
        componentArrays.push_back(m_componentManager.GetComponentArray(index).value());
    m_combinations.insert({description.name, {componentArrays}});
}

void Manager::execute(const event::Event &event)
{
    for (const auto &[name, description]: m_systems) {
        if (event.description.name == description.eventListener.name) executeOne(description, event);
    }
}

Manager::const_iterator Manager::begin() const { return m_systems.begin(); }

Manager::const_iterator Manager::end() const { return m_systems.end(); }

std::vector<component::Manager::ComponentId> Manager::getComponentsId(const std::vector<std::string> &components)
{
    std::vector<component::Manager::ComponentId> componentsId;

    for (const auto &component: components) {
        auto componentId = m_componentManager.GetComponentId(component);
        if (!componentId.has_value()) throw MissingComponent(component);
        componentsId.push_back(componentId.value());
    }
    return componentsId;
}

void Manager::executeOne(const Description &description, const event::Event &event)
{
    if (event.entities.size() != description.eventComponents.size())
        throw std::logic_error("This system expect " + std::to_string(description.eventComponents.size()) + " entity.");

    event::EntityComponents entitiesComponents;
    for (std::size_t i = 0; i < event.entities.size(); i++) {
        std::vector<component::ComponentRef> entityComponents;
        for (const auto index: getComponentsId(description.eventComponents[i])) {
            if (!m_componentManager.GetComponent(event.entities[i], index).has_value()) return;
            entityComponents.emplace_back(m_componentManager.GetComponentArray(index).value().get(), event.entities[i]);
        }
        entitiesComponents.push_back(entityComponents);
    }

    event::EventWithComponent entityComponents{.event = event, .components = entitiesComponents};
    description.system(description, m_combinations.at(description.name), entityComponents);
}

}    // namespace pivot::ecs::systems