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
class Index
{
public:
    void registerSystem(const Description &description);

    std::optional<Description> getDescription(const std::string &systemName) const;

    using const_iterator = std::map<std::string, Description>::const_iterator;
    const_iterator begin() const;
    const_iterator end() const;

    std::vector<std::string> getAllSystemsNames() const;

    struct DuplicateError : public std::logic_error {
        DuplicateError(const std::string &systemName);

        std::string systemName;
    };

private:
    std::map<std::string, Description> m_descriptionByName;
};

class GlobalIndex : private Index
{
public:
    void registerSystem(const Description &description);

    std::optional<Description> getDescription(const std::string &componentName);
    
    Index::const_iterator begin();
    Index::const_iterator end();
    std::vector<std::string> getAllSystemsNames();

    static GlobalIndex &getSingleton();

private:
    std::atomic<bool> m_read_only;
    std::mutex m_mutex;

    void lockReadOnly();
};

}    // namespace pivot::ecs::systems