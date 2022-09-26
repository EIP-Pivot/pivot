#include "pivot/ecs/Core/Systems/manager.hxx"
#include "pivot/pivot.hxx"

namespace pivot::ecs::systems
{
Manager::MissingComponent::MissingComponent(const std::string &componentName)
    : std::runtime_error("Missing component: " + componentName), componentName(componentName)
{
}

Manager::MissingSystem::MissingSystem(const std::string &systemName)
    : std::runtime_error("Missing system: " + systemName), systemName(systemName)
{
}

void Manager::useSystem(const Description &description)
{
    DEBUG_FUNCTION
    if (m_systems.contains(description.name)) throw EcsException("System already use.");
    m_systems.insert({description.name, description});

    std::vector<std::reference_wrapper<component::IComponentArray>> componentArrays;
    for (const auto index: getComponentsId(description.systemComponents))
        componentArrays.push_back(m_componentManager.GetComponentArray(index));
    m_combinations.insert({description.name, {componentArrays}});
}

std::vector<event::Event> Manager::execute(const event::Event &event)
{
    DEBUG_FUNCTION
    std::vector<event::Event> childEvent;

    for (const auto &[name, description]: m_systems) {
        if (event.description.name == description.eventListener.name) {
            auto events = executeOne(description, event);
            childEvent.insert(childEvent.end(), events.begin(), events.end());
        }
    }
    return childEvent;
}

Manager::const_iterator Manager::begin() const { return m_systems.begin(); }

Manager::const_iterator Manager::end() const { return m_systems.end(); }

std::vector<component::Manager::ComponentId> Manager::getComponentsId(const std::vector<std::string> &components)
{
    DEBUG_FUNCTION
    std::vector<component::Manager::ComponentId> componentsId;

    for (const auto &component: components) {
        auto componentId = m_componentManager.GetComponentId(component);
        if (!componentId.has_value()) throw MissingComponent(component);
        componentsId.push_back(componentId.value());
    }
    return componentsId;
}

std::vector<event::Event> Manager::executeOne(const Description &description, const event::Event &event)
{
    DEBUG_FUNCTION
    if (event.entities.size() != description.eventComponents.size())
        throw std::logic_error("This system expect " + std::to_string(description.eventComponents.size()) + " entity.");

    event::EntityComponents entitiesComponents;
    for (std::size_t i = 0; i < event.entities.size(); i++) {
        std::vector<component::ComponentRef> entityComponents;
        for (const auto index: getComponentsId(description.eventComponents[i])) {
            if (!m_componentManager.GetComponent(event.entities[i], index).has_value()) return {};
            entityComponents.emplace_back(m_componentManager.GetComponentArray(index), event.entities[i]);
        }
        entitiesComponents.push_back(entityComponents);
    }

    event::EventWithComponent entityComponents{.event = event, .components = entitiesComponents};
    return description.system(description, m_combinations.at(description.name), entityComponents);
}

}    // namespace pivot::ecs::systems
