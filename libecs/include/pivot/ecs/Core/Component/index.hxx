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
 *
 * The type based access is only a convenience, and should not be relied upon to
 * get every possible components, as most components are dynamic and have no
 * corresponding C++ type.
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

    // /// Registers a component Description by its name, and registered the name by its type
    // template <typename T>
    // void registerComponentWithType(const Description &description)
    // {
    //     auto index = std::type_index(typeid(T));
    //     if (m_type_to_name.contains(index) || m_components.contains(description.name)) {
    //         throw DuplicateError(description.name);
    //     }
    //     this->registerComponent(description);
    //     m_type_to_name.insert({index, description.name});
    // }

    /// Get the name of a component by its type if it was registered
    // template <typename T>
    // std::optional<std::string> getComponentNameByType()
    // {
    //     auto it = m_type_to_name.find(std::type_index(typeid(T)));
    //     if (it == m_type_to_name.end()) {
    //         return std::nullopt;
    //     } else {
    //         return std::make_optional(it->second);
    //     }
    // }

private:
    std::map<std::string, Description> m_components;
    // std::unordered_map<std::type_index, std::string> m_type_to_name;
};

/** \brief A variant of the component Index supporting concurrent accesses
 *
 * The GlobalIndex stores component registered through the global component
 * registration method described in description_helpers.hxx. As this methods
 * registers components in a global Index, this Index needs to handle concurrent
 * accesses.
 *
 * The GlobalIndex handles those concurrent accesses by synchronizing explicitely
 * every component registration. To prevent a performance penalty once every
 * global component registration has been performed, the first read in the
 * GlobalIndex puts it into a readonly mode, where components cannot be
 * registered anymore.
 */
class GlobalIndex : private Index
{

public:
    /** \brief See Index::registerComponent()
     *
     * Throws if the GlobalIndex is in read only mode
     */
    void registerComponent(const Description &description);
    /// Locks the index in readonly mode. See Index::getDescription()
    std::optional<Description> getDescription(const std::string &componentName);
    /// Locks the index in readonly mode. See Index::getAllComponentsNames()
    std::vector<std::string> getAllComponentsNames();
    /// Locks the index in readonly mode. See Index::begin()
    Index::const_iterator begin();
    /// Locks the index in readonly mode. See Index::end()
    Index::const_iterator end();

    // /** \brief See Index::registerComponentWithType()
    //  *
    //  * Throws if the GlobalIndex is in read only mode
    //  */
    // template <typename T>
    // void registerComponentWithType(const Description &description)
    // {

    //     if (m_read_only) { throw std::logic_error("Cannot modify global component index after program started"); }

    //     const std::lock_guard<std::mutex> guard(m_mutex);

    //     this->Index::registerComponentWithType<T>(description);
    // }

    // /// Locks the index in readonly mode. See Index::getComponentNameByType()
    // template <typename T>
    // std::optional<std::string> getComponentNameByType()
    // {
    //     this->lockReadOnly();
    //     return this->Index::getComponentNameByType<T>();
    // }

    /// Gives access to the global GlobalIndex instance, used to register
    /// components globally.
    static GlobalIndex &getSingleton();

private:
    std::atomic<bool> m_read_only;
    std::mutex m_mutex;

    void lockReadOnly();
};

}    // namespace pivot::ecs::component
