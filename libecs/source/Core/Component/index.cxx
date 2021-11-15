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

const Description &Index::getDescription(const std::string &componentName) const
{
    return m_components.at(componentName);
}

std::vector<std::string> Index::getAllComponentsNames() const
{
    std::vector<std::string> names;
    for (auto &[key, value]: m_components) { names.push_back(key); }
    return names;
}

void GlobalIndex::registerComponent(const Description &description)
{
    if (m_read_only) { throw std::logic_error("Cannot modify global component index after program started"); }

    const std::lock_guard<std::mutex> guard(m_mutex);

    this->Index::registerComponent(description);
}

const Description &GlobalIndex::getDescription(const std::string &componentName)
{
    if (!m_read_only) {
        const std::lock_guard<std::mutex> guard(m_mutex);
        m_read_only.store(true);
    }

    return this->Index::getDescription(componentName);
}
}    // namespace pivot::ecs::component
