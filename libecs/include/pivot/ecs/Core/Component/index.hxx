#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <typeindex>

#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::ecs::component
{

/** \brief An index of Description
 *
 * This call contains a list of component Description which can be registered by their name or type, and then retrieved
 * by their name or type.
 */
class Index
{
public:
    /// Registers an component Description in the index of later use
    void registerComponent(const Description &description);

    /// Get the Description of a component if it exists
    std::optional<Description> getDescription(const std::string &componentName) const;

    /// Constant iterator over every component in the index
    using const_iterator = std::map<std::string, Description>::const_iterator;
    const_iterator begin() const;    ///< Begin iterator
    const_iterator end() const;      ///< End iterator

    /// Returns the list of the names of all the components registered in the index
    std::vector<std::string> getAllComponentsNames() const;

    /// Error thrown when two components with the same name are registered
    struct DuplicateError : public std::logic_error {
        /// Created a DuplicateError based on a component's name
        DuplicateError(const std::string &componentName);

        /// Name of the duplicated component
        std::string componentName;
    };

private:
    std::map<std::string, Description> m_components;
    // std::unordered_map<std::type_index, std::string> m_type_to_name;
};

}    // namespace pivot::ecs::component
