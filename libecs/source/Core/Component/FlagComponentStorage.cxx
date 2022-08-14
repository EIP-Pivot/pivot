#include <pivot/ecs/Core/Component/FlagComponentStorage.hxx>
#include <pivot/ecs/Core/Component/error.hxx>

namespace pivot::ecs::component
{

const Description &FlagComponentStorage::getDescription() const { return m_description; }

std::optional<data::Value> FlagComponentStorage::getValueForEntity(Entity entity) const
{
    if (m_entity_having_component.contains(entity)) { return std::make_optional<data::Value>(data::Void{}); }
    return std::nullopt;
}

bool FlagComponentStorage::entityHasValue(Entity entity) const { return m_entity_having_component.contains(entity); }

void FlagComponentStorage::setValueForEntity(Entity entity, std::optional<data::Value> value)
{
    if (value.has_value()) {
        if (!std::holds_alternative<data::Void>(*value)) {
            throw InvalidComponentValue(m_description.name, m_description.type, value->type());
        }
        m_entity_having_component.insert(entity);
        m_max_entity = std::max(m_max_entity, entity);
    } else {
        m_entity_having_component.erase(entity);
    }
}

Entity FlagComponentStorage::maxEntity() const { return m_max_entity; }

const std::set<Entity> &FlagComponentStorage::getData() { return m_entity_having_component; }
}    // namespace pivot::ecs::component
