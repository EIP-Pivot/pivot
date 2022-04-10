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
}    // namespace pivot::ecs::component
