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
    if (m_systems.contains(description.name)) { throw DuplicateError(description.name); }
    m_systems.insert({description.name, description});
}

std::optional<Description> Index::getDescription(const std::string &systemName) const
{
    auto it = m_systems.find(systemName);
    if (it == m_systems.end()) {
        return std::nullopt;
    } else {
        return std::make_optional(it->second);
    }
}

std::vector<std::string> Index::getAllSystemsNames() const
{
    std::vector<std::string> names;
    for (auto &[key, value]: m_systems) { names.push_back(key); }
    return names;
}
}