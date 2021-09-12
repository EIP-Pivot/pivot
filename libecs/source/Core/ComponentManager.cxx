#include "pivot/ecs/Core/ComponentManager.hxx"

void ComponentManager::EntityDestroyed(Entity entity)
{
    for (auto const &pair: mComponentArrays) {
        auto const &component = pair.second;

        component->EntityDestroyed(entity);
    }
}