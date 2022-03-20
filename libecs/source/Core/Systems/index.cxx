#include "pivot/ecs/Core/Systems/index.hxx"

namespace pivot::ecs::systems
{
Index::DuplicateError::DuplicateError(const std::string &systemName)
    : std::logic_error("Duplicated system name: " + systemName), systemName(systemName)
{
}

void Index::registerSystem(const Description &description)
{
    description.validate();
    if (m_descriptionByName.contains(description.name)) { throw DuplicateError(description.name); }

    m_descriptionByName.insert({description.name, description});
}

std::optional<Description> Index::getDescription(const std::string &systemName) const
{
    auto it = m_descriptionByName.find(systemName);
    if (it == m_descriptionByName.end()) {
        return std::nullopt;
    } else {
        return std::make_optional(it->second);
    }
}

std::vector<std::string> Index::getAllSystemsNames() const
{
    std::vector<std::string> names;
    for (auto &[key, value]: m_descriptionByName) { names.push_back(key); }
    return names;
}

Index::const_iterator Index::begin() const { return m_descriptionByName.begin(); }

Index::const_iterator Index::end() const { return m_descriptionByName.end(); }

void GlobalIndex::registerSystem(const Description &description)
{
    if (m_read_only) { throw std::logic_error("Cannot modify global system index after program started"); }

    const std::lock_guard<std::mutex> guard(m_mutex);

    this->Index::registerSystem(description);
}

std::optional<Description> GlobalIndex::getDescription(const std::string &componentName)
{
    this->lockReadOnly();
    return this->Index::getDescription(componentName);
}

std::vector<std::string> GlobalIndex::getAllSystemsNames()
{

    this->lockReadOnly();
    return this->Index::getAllSystemsNames();
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
}    // namespace pivot::ecs::systems