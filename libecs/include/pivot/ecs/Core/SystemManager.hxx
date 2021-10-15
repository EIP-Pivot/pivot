#pragma once

#include "pivot/ecs/Core/types.hxx"
#include "pivot/ecs/Core/System.hxx"
#include "pivot/ecs/Core/EcsException.hxx"
#include <memory>
#include <unordered_map>

/*! \cond
 */
class SystemManager
{
public:
    template <typename T>
    std::shared_ptr<T> RegisterSystem()
    {
        const char *typeName = typeid(T).name();
        if (mSystems.contains(typeName))
            throw EcsException("Registering system more than once.");

        auto system = std::make_shared<T>();
        mSystems.insert({typeName, system});
        return system;
    }

    template <typename T>
    void SetSignature(Signature signature)
    {
        const char *typeName = typeid(T).name();
        if (!mSystems.contains(typeName))
            throw EcsException("System used before registered.");

        mSignatures.insert({typeName, signature});
    }

    template <typename T>
    void setEntityToSystem(Entity entity)
    {
        const char *typeName = typeid(T).name();
        mSystems[typeName]->mEntities.insert(entity);
    }

    template <typename T>
    bool hasSystem()
    {
        const char *typeName = typeid(T).name();
        return mSystems.contains(typeName);
    }

    void EntityDestroyed(Entity entity);
    void EntitySignatureChanged(Entity entity, Signature entitySignature);

private:
    std::unordered_map<const char *, Signature> mSignatures{};
    std::unordered_map<const char *, std::shared_ptr<System>> mSystems{};
};
/*! \endcond
 */