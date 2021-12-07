#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <typeindex>

#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::ecs::component
{
class Index
{
public:
    void registerComponent(const Description &description);

    std::optional<Description> getDescription(const std::string &componentName) const;

    std::vector<std::string> getAllComponentsNames() const;

    struct DuplicateError : public std::logic_error {
        DuplicateError(const std::string &componentName);

        std::string componentName;
    };

    template <typename T>
    void registerComponentWithType(const Description &description)
    {
        auto index = std::type_index(typeid(T));
        if (m_type_to_name.contains(index) || m_components.contains(description.name)) {
            throw DuplicateError(description.name);
        }
        this->registerComponent(description);
        m_type_to_name.insert({index, description.name});
    }

    template <typename T>
    std::optional<std::string> getComponentNameByType()
    {
        auto it = m_type_to_name.find(std::type_index(typeid(T)));
        if (it == m_type_to_name.end()) {
            return std::nullopt;
        } else {
            return std::make_optional(it->second);
        }
    }

private:
    std::map<std::string, Description> m_components;
    std::unordered_map<std::type_index, std::string> m_type_to_name;
};

class GlobalIndex : private Index
{

public:
    void registerComponent(const Description &description);
    std::optional<Description> getDescription(const std::string &componentName);

    template <typename T>
    void registerComponentWithType(const Description &description)
    {

        if (m_read_only) { throw std::logic_error("Cannot modify global component index after program started"); }

        const std::lock_guard<std::mutex> guard(m_mutex);

        this->Index::registerComponentWithType<T>(description);
    }

    template <typename T>
    std::optional<std::string> getComponentNameByType()
    {
        this->lockReadOnly();
        return this->Index::getComponentNameByType<T>();
    }

    static GlobalIndex &getSingleton();

private:
    std::atomic<bool> m_read_only;
    std::mutex m_mutex;

    void lockReadOnly();
};

}    // namespace pivot::ecs::component
