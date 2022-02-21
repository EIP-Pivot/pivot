#pragma once

#include "pivot/ecs/Core/Event/description.hxx"

#include <atomic>
#include <mutex>
#include <vector>

namespace pivot::ecs::event
{

class Index
{
public:
    void registerEvent(const Description &description);

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

class GlobalIndex : private Index
{
public:
    void registerEvent(const Description &description);

    std::optional<Description> getDescription(const std::string &eventName);

    Index::const_iterator begin();
    Index::const_iterator end();
    std::vector<std::string> getAllEventsNames();

    static GlobalIndex &getSingleton();

private:
    std::atomic<bool> m_read_only;
    std::mutex m_mutex;

    void lockReadOnly();
};

}    // namespace pivot::ecs::event