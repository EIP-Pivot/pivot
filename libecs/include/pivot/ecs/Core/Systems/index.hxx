#pragma once

#include "pivot/ecs/Core/Component/manager.hxx"
#include "pivot/ecs/Core/EntityManager.hxx"
#include "pivot/ecs/Core/Systems/description.hxx"

#include <atomic>
#include <map>
#include <mutex>
#include <optional>
#include <typeindex>

namespace pivot::ecs::systems
{
/** \brief An index of Description
 *
 * This call contains a list of system Description which can be registered by their name, and then retrieved
 * by their name.
 */
class Index
{
public:
    /// Registers an system Description in the index of later use
    void registerSystem(const Description &description);
    /// Get the Description of a system if it exists
    std::optional<Description> getDescription(const std::string &systemName) const;
    /// Constant iterator over every system in the index
    using const_iterator = std::map<std::string, Description>::const_iterator;
    const_iterator begin() const;    ///< Begin iterator
    const_iterator end() const;      ///< End iterator
    /// Returns the list of the names of all the system registered in the index
    std::vector<std::string> getAllSystemsNames() const;

    /// Error thrown when two system with the same name are registered
    struct DuplicateError : public std::logic_error {
        /// Created a DuplicateError based on a systems's name
        DuplicateError(const std::string &systemName);
        /// Name of the duplicated system
        std::string systemName;
    };

private:
    std::map<std::string, Description> m_descriptionByName;
};

}    // namespace pivot::ecs::systems
