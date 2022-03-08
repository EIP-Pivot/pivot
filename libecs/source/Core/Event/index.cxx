#include "pivot/ecs/Core/Event/index.hxx"

namespace pivot::ecs::event
{

Index::DuplicateError::DuplicateError(const std::string &eventName)
    : std::logic_error("Duplicated event name: " + eventName), eventName(eventName)
{
}

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

void GlobalIndex::registerEvent(const Description &description)
{
    if (m_read_only) { throw std::logic_error("Cannot modify global event index after program started"); }

    const std::lock_guard<std::mutex> guard(m_mutex);

    this->Index::registerEvent(description);
}

std::optional<Description> GlobalIndex::getDescription(const std::string &eventName)
{
    this->lockReadOnly();
    return this->Index::getDescription(eventName);
}

std::vector<std::string> GlobalIndex::getAllEventsNames()
{

    this->lockReadOnly();
    return this->Index::getAllEventsNames();
}

void GlobalIndex::lockReadOnly()
{
    if (!m_read_only) {
        const std::lock_guard<std::mutex> guard(m_mutex);
        m_read_only.store(true);
    }
}

namespace
{
    static std::unique_ptr<GlobalIndex> singleton = nullptr;
}

GlobalIndex &GlobalIndex::getSingleton()
{
    if (!singleton) singleton = std::make_unique<GlobalIndex>();
    return *singleton;
}

Index::const_iterator GlobalIndex::begin()
{
    this->lockReadOnly();
    return this->Index::begin();
}

Index::const_iterator GlobalIndex::end()
{
    this->lockReadOnly();
    return this->Index::end();
}

}    // namespace pivot::ecs::event