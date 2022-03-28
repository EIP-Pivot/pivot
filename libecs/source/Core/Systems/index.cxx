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
}    // namespace pivot::ecs::systems
