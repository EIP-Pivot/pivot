#include <atomic>
#include <mutex>
#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::ecs::component
{
class Index
{
public:
    void registerComponent(const Description &description);

    const Description &getDescription(const std::string &componentName) const;

    struct DuplicateError : public std::logic_error {
        DuplicateError(const std::string &componentName);

        std::string componentName;
    };

private:
    std::map<std::string, Description> m_components;
};

class GlobalIndex : private Index
{

public:
    void registerComponent(const Description &description);
    const Description &getDescription(const std::string &componentName);

private:
    std::atomic<bool> m_read_only;
    std::mutex m_mutex;
};
}    // namespace pivot::ecs::component
