#include "pivot/ecs/Core/EntityManager.hxx"

EntityManager::EntityManager()
{
    for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
        mAvailableEntities.push(entity);
}

Entity EntityManager::CreateEntity()
{
    if (mLivingEntityCount >= MAX_ENTITIES)
        throw EcsException("Too many entities in existence.");

    Entity id = mAvailableEntities.front();
    mAvailableEntities.pop();
    ++mLivingEntityCount;

    return id;
}

void EntityManager::DestroyEntity(Entity entity)
{
    if (entity >= MAX_ENTITIES)
        throw EcsException("Entity out of range.");

    mSignatures[entity].reset();
    mAvailableEntities.push(entity);
    --mLivingEntityCount;
}

void EntityManager::SetSignature(Entity entity, Signature signature)
{
    if (entity >= MAX_ENTITIES)
        throw EcsException("Entity out of range.");

    mSignatures[entity] = signature;
}

Signature EntityManager::GetSignature(Entity entity)
{
    if (entity >= MAX_ENTITIES)
        throw EcsException("Entity out of range.");

    return mSignatures[entity];
}