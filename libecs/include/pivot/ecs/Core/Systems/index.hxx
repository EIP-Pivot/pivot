#pragma once

#include <optional>
#include <typeindex>
#include <map>

#include "pivot/ecs/Core/Systems/description.hxx"

namespace pivot::ecs::systems
{
class Index
{
public:
    void registerSystem(const Description &description);

    std::optional<Description> getDescription(const std::string &systemName) const;

    std::vector<std::string> getAllSystemsNames() const;

    struct DuplicateError : public std::logic_error {
        DuplicateError(const std::string &systemName);

        std::string systemName;
    };

private:
    std::map<std::string, Description> m_systems;
};
}