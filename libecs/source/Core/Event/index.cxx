#include "pivot/ecs/Core/Event/index.hxx"

namespace pivot::ecs::event
{

void Index::registerEvent(const Description &description)
{
    description.validate();
    if (m_events.contains(description.name)) { throw DuplicateError(description.name); }
    m_events.insert({description.name, description});
}

std::optional<Description> Index::getDescription(const std::string &eventName) const
{
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
    std::vector<std::string> names;
    names.reserve(m_events.size());
    for (auto &[key, value]: m_events) { names.push_back(key); }
    return names;
}

}