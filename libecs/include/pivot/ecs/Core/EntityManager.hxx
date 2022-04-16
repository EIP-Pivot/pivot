#pragma once

#include "pivot/ecs/Core/EcsException.hxx"
#include "pivot/ecs/Core/types.hxx"
#include <array>
#include <queue>
#include <unordered_map>

/*! \cond
 */
class EntityManager
{
public:
    EntityManager();
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    void SetSignature(Entity entity, Signature signature);
    Signature GetSignature(Entity entity);
    std::unordered_map<Entity, Signature> getEntities() const;
    uint32_t getLivingEntityCount();

private:
    std::queue<Entity> mAvailableEntities{};
    std::unordered_map<Entity, Signature> mEntities;
    uint32_t mLivingEntityCount{};
};
/*! \endcond
 */
