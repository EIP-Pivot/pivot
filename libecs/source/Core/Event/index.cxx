#include "pivot/ecs/Core/Event/index.hxx"

#include "pivot/pivot.hxx"

namespace pivot::ecs::event
{

Index::DuplicateError::DuplicateError(const std::string &eventName)
    : std::logic_error("Duplicated event name: " + eventName), eventName(eventName)
{
}

void Index::registerEvent(const Description &description)
{
    PROFILE_FUNCTION();
    description.validate();
    if (m_events.contains(description.name)) { throw DuplicateError(description.name); }
    m_events.insert({description.name, description});
}

std::optional<Description> Index::getDescription(const std::string &eventName) const
{
    PROFILE_FUNCTION();
    auto it = m_events.find(eventName);
    if (it == m_events.end()) {
        return std::nullopt;
    } else {
        return std::make_optional(it->second);
    }
}

Index::const_iterator Index::begin() const { return m_events.begin(); }

Index::const_iterator Index::end() const { return m_events.end(); }

std::vector<std::string> Index::getAllEventsNames() const
{
    PROFILE_FUNCTION();
    std::vector<std::string> names;
    names.reserve(m_events.size());
    for (auto &[key, value]: m_events) { names.push_back(key); }
    return names;
}
}    // namespace pivot::ecs::event
