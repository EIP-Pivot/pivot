#pragma once

#include <pivot/ecs/Core/Systems/description.hxx>
#include <pivot/graphics/AssetStorage/AssetStorage.hxx>

namespace pivot::builtins::systems
{
const pivot::ecs::systems::Description makeCollisionSystem(const pivot::graphics::AssetStorage &assetStorage);

namespace details
{

    struct EntityAABB {
        glm::vec3 low;
        glm::vec3 high;
        Entity entity;
    };

    std::vector<std::pair<Entity, Entity>> getEntityCollisions(std::span<const EntityAABB> entityAABB);
}    // namespace details
}    // namespace pivot::builtins::systems
