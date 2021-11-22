#pragma once
#include <array>
#include <optional>
#include <unordered_map>

#include <pivot/ecs/Core/Component/description.hxx>
#include <pivot/ecs/Core/types.hxx>

namespace pivot::ecs::component
{
class IComponentArray
{
public:
    virtual ~IComponentArray() = default;

    virtual const Description &getDescription() const = 0;
    virtual std::optional<std::any> getValueForEntity(Entity entity) const = 0;
    virtual std::optional<std::any> getRefForEntity(Entity entity) = 0;
    virtual void setValueForEntity(Entity entity, std::optional<std::any>) = 0;
};
}    // namespace pivot::ecs::component
