#pragma once

#include "pivot/ecs/Core/types.hxx"
#include "pivot/ecs/Core/EcsException.hxx"
#include <queue>
#include <array>

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

private:
    std::queue<Entity> mAvailableEntities{};
    std::array<Signature, MAX_ENTITIES> mSignatures{};
    uint32_t mLivingEntityCount{};
};
/*! \endcond
 */