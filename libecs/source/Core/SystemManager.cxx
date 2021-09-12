#include "pivot/ecs/Core/SystemManager.hxx"

void SystemManager::EntityDestroyed(Entity entity)
{
    for (auto const &pair: mSystems) {
        auto const &system = pair.second;

        system->mEntities.erase(entity);
    }
}

void SystemManager::EntitySignatureChanged(Entity entity, Signature entitySignature)
{
    for (auto const &pair: mSystems) {
        auto const &type = pair.first;
        auto const &system = pair.second;
        auto const &systemSignature = mSignatures[type];

        if ((entitySignature & systemSignature) == systemSignature) {
            system->mEntities.insert(entity);
        } else {
            system->mEntities.erase(entity);
        }
    }
}