#pragma once

#include "pivot/ecs/Core/Event/description.hxx"

#include <atomic>
#include <mutex>
#include <vector>

namespace pivot::ecs::event
{

/** \brief An index of Description
 *
 * This call contains a list of event Description which can be registered by their name, and then retrieved
 * by their name.
 */
class Index
{
public:
    /// Registers an event Description in the index of later use
    void registerEvent(const Description &description);

    /// Get the Description of a event if it exists
    std::optional<Description> getDescription(const std::string &eventName) const;

    /// Constant iterator over every event in the index
    using const_iterator = std::map<std::string, Description>::const_iterator;
    const_iterator begin() const;    ///< Begin iterator
    const_iterator end() const;      ///< End iterator

    /// Returns the list of the names of all the event registered in the index
    std::vector<std::string> getAllEventsNames() const;

    /// Error thrown when two events with the same name are registered
    struct DuplicateError : public std::logic_error {
        /// Created a DuplicateError based on a event's name
        DuplicateError(const std::string &eventName);

        /// Name of the duplicated event
        std::string eventName;
    };

private:
    std::map<std::string, Description> m_events;
};

/** \brief A variant of the event Index supporting concurrent accesses
 *
 * The GlobalIndex handles those concurrent accesses by synchronizing explicitely
 * every event registration. To prevent a performance penalty once every
 * global event registration has been performed, the first read in the
 * GlobalIndex puts it into a readonly mode, where events cannot be
 * registered anymore.
 */
class GlobalIndex : private Index
{
public:
    /** \brief See Index::registerEvent()
     *
     * Throws if the GlobalIndex is in read only mode
     */
    void registerEvent(const Description &description);
    /// Locks the index in readonly mode. See Index::getDescription()
    std::optional<Description> getDescription(const std::string &eventName);
    /// Locks the index in readonly mode. See Index::begin()
    Index::const_iterator begin();
    /// Locks the index in readonly mode. See Index::end()
    Index::const_iterator end();

    /// Get a list of event name
    std::vector<std::string> getAllEventsNames();

    /// Gives access to the global GlobalIndex instance, used to register
    /// events globally.
    static GlobalIndex &getSingleton();

private:
    std::atomic<bool> m_read_only;
    std::mutex m_mutex;

    void lockReadOnly();
};

}    // namespace pivot::ecs::event