#include "pivot/ecs/Core/Systems/index.hxx"

namespace pivot::ecs::systems
{
Index::DuplicateError::DuplicateError(const std::string &systemName)
    : std::logic_error("Duplicated system name: " + systemName), systemName(systemName)
{
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

std::optional<std::function<void(component::Manager &, EntityManager &)>>
Index::getSystemByName(const std::string &systemName)
{
    auto it = m_systemsByName.find(systemName);
    if (it == m_systemsByName.end()) {
        return std::nullopt;
    } else {
        return std::make_optional(std::ref(it->second));
    }
}

std::optional<std::function<void(component::Manager &, EntityManager &)>>
Index::getSystemByDescription(const Description &description)
{
    return getSystemByName(description.name);
}


std::vector<std::string> Index::getAllSystemsNames() const
{
    std::vector<std::string> names;
    for (auto &[key, value]: m_descriptionByName) { names.push_back(key); }
    return names;
}

Index::const_iterator Index::begin() const { return m_descriptionByName.begin(); }
Index::const_iterator Index::end() const { return m_descriptionByName.end(); }

std::optional<Description> GlobalIndex::getDescription(const std::string &componentName)
{
    this->lockReadOnly();
    return this->Index::getDescription(componentName);
}

std::optional<std::function<void(component::Manager &, EntityManager &)>>
GlobalIndex::getSystemByName(const std::string &systemName)
{
    this->lockReadOnly();
    return this->Index::getSystemByName(systemName);
}
std::optional<std::function<void(component::Manager &, EntityManager &)>>
GlobalIndex::getSystemByDescription(const Description &description)
{
    this->lockReadOnly();
    return this->Index::getSystemByDescription(description);
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
}