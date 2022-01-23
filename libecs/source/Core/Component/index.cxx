#include <Logger.hpp>

#include <pivot/ecs/Core/Component/index.hxx>

namespace pivot::ecs::component
{

Index::DuplicateError::DuplicateError(const std::string &componentName)
    : std::logic_error("Duplicated component name: " + componentName), componentName(componentName)
{
}

void Index::registerComponent(const Description &description)
{
    description.validate();
    if (m_components.contains(description.name)) { throw DuplicateError(description.name); }
    m_components.insert({description.name, description});
}

std::optional<Description> Index::getDescription(const std::string &componentName) const
{
    auto it = m_components.find(componentName);
    if (it == m_components.end()) {
        return std::nullopt;
    } else {
        return std::make_optional(it->second);
    }
}

Index::const_iterator Index::begin() const { return m_components.begin(); }
Index::const_iterator Index::end() const { return m_components.end(); }

std::vector<std::string> Index::getAllComponentsNames() const
{
    std::vector<std::string> names;
    names.reserve(m_components.size());
    for (auto &[key, value]: m_components) { names.push_back(key); }
    return names;
}

void GlobalIndex::registerComponent(const Description &description)
{
    const std::lock_guard<std::mutex> guard(m_mutex);

    if (m_read_only) { throw std::logic_error("Cannot modify global component index after program started"); }

    this->Index::registerComponent(description);
}

std::optional<Description> GlobalIndex::getDescription(const std::string &componentName)
{
    this->lockReadOnly();
    return this->Index::getDescription(componentName);
}

std::vector<std::string> GlobalIndex::getAllComponentsNames()
{
    this->lockReadOnly();
    return this->Index::getAllComponentsNames();
}

void GlobalIndex::lockReadOnly()
{
    logger.info() << "Locking global component index in read-only mode";
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

}    // namespace pivot::ecs::component
