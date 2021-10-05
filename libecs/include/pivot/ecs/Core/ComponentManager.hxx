#pragma once

#include "pivot/ecs/Core/ComponentArray.hxx"
#include "pivot/ecs/Core/EcsException.hxx"
#include "pivot/ecs/Core/types.hxx"
#include <any>
#include <memory>

/*! \cond
 */
class ComponentManager
{
public:
    template <typename T>
    void RegisterComponent()
    {
        const char *typeName = typeid(T).name();

        if (mComponentTypes.contains(typeName))
            throw EcsException("Registering component type more than once.");

        mComponentTypes.insert({typeName, mNextComponentType});
        mComponentArrays.insert({typeName, std::make_shared<ComponentArray<T>>()});

        ++mNextComponentType;
    }

    template <typename T>
    ComponentType GetComponentType()
    {
        const char *typeName = typeid(T).name();
        if (!mComponentTypes.contains(typeName))
            throw EcsException("Component not registered before use.");

        return mComponentTypes[typeName];
    }

    std::unordered_map<const char *, ComponentType> getComponentsTypes();

    template <typename T>
    void AddComponent(Entity entity, T component)
    {
        GetComponentArray<T>()->InsertData(entity, component);
    }

    template <typename T>
    void RemoveComponent(Entity entity)
    {
        GetComponentArray<T>()->RemoveData(entity);
    }

    template <typename T>
    T &GetComponent(Entity entity)
    {
        return GetComponentArray<T>()->GetData(entity);
    }

    void EntityDestroyed(Entity entity);

private:
    std::unordered_map<const char *, ComponentType> mComponentTypes{};
    std::unordered_map<const char *, std::shared_ptr<IComponentArray>> mComponentArrays{};
    ComponentType mNextComponentType{};

    template <typename T>
    std::shared_ptr<ComponentArray<T>> GetComponentArray()
    {
        const char *typeName = typeid(T).name();
        if (!mComponentTypes.contains(typeName))
            throw EcsException("Component not registered before use.");

        return std::static_pointer_cast<ComponentArray<T>>(mComponentArrays[typeName]);
    }
};
/*! \endcond
 */