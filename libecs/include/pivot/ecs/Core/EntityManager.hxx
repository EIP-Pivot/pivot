#pragma once

#include "pivot/ecs/Core/types.hxx"
#include "pivot/ecs/Core/EcsException.hxx"
#include <queue>
#include <array>

class EntityManager
{
public:
    EntityManager()
    {
        for (Entity entity = 0; entity < MAX_ENTITIES; ++entity) { mAvailableEntities.push(entity); }
    }

    Entity CreateEntity()
    {
        if (mLivingEntityCount >= MAX_ENTITIES)
            throw EcsException("Too many entities in existence.");

        Entity id = mAvailableEntities.front();
        mAvailableEntities.pop();
        ++mLivingEntityCount;

        return id;
    }

    void DestroyEntity(Entity entity)
    {
        if (entity >= MAX_ENTITIES)
            throw EcsException("Entity out of range.");

        mSignatures[entity].reset();
        mAvailableEntities.push(entity);
        --mLivingEntityCount;
    }

    void SetSignature(Entity entity, Signature signature)
    {
        if (entity >= MAX_ENTITIES)
            throw EcsException("Entity out of range.");

        mSignatures[entity] = signature;
    }

    Signature GetSignature(Entity entity)
    {
        if (entity >= MAX_ENTITIES)
            throw EcsException("Entity out of range.");

        return mSignatures[entity];
    }

private:
    std::queue<Entity> mAvailableEntities{};
    std::array<Signature, MAX_ENTITIES> mSignatures{};
    uint32_t mLivingEntityCount{};
};