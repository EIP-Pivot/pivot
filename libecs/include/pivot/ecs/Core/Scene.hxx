#pragma once

#include "pivot/ecs/Core/types.hxx"
#include "pivot/ecs/Core/ComponentManager.hxx"
#include "pivot/ecs/Core/EntityManager.hxx"
#include "pivot/ecs/Core/SystemManager.hxx"
#include "pivot/ecs/Core/EventManager.hxx"
#include <memory>

#include "pivot/ecs/Components/Camera.hxx"
#include "pivot/ecs/Components/Tag.hxx"

/// @class Coordinator
///
/// @brief The main class of the ecs
///
/// You only need this class to manage all the Entity component syteme
/// @code
/// // Initializes the coordinator before its use
/// // (Temporary) The coordinator is declared as a global variables gCoordinator
/// gCoordinator.Init();
/// @endcode

class IScene {
public:

protected:
    virtual void OnCreate() {};
    virtual void OnTick() {};
    virtual void OnPause() {};
    virtual void OnResume() {};
    virtual void OnStop() {};
};

class Scene : public IScene
{
public:
    Scene(std::string sceneName = "Scene"): name(sceneName){};
    void Init();

    std::string getName();

    // Entity methods

    /// Create a new entity
    /// @return Entity New entity
    Entity CreateEntity();
    Entity CreateEntity(std::string name);

    /// @param[in] entity  Entity to remove.
    void DestroyEntity(Entity entity);

    Signature getSignature(Entity entity);

    uint32_t getLivingEntityCount();

    // Component methods

    /// Register a new component before its usage
    /// @code
    /// gCoordinator.RegisterComponent<{YourComponent}>();
    /// @endcode
    template <typename T>
    void RegisterComponent()
    {
        mComponentManager->RegisterComponent<T>();
    }

    /// Add a component to Entity
    /// @code
    /// gCoordinator.AddComponent<{YourComponent}>({YourEntity});
    /// @endcode
    template <typename T>
    void AddComponent(Entity entity, T component)
    {
        mComponentManager->AddComponent<T>(entity, component);

        auto signature = mEntityManager->GetSignature(entity);
        signature.set(mComponentManager->GetComponentType<T>(), true);
        mEntityManager->SetSignature(entity, signature);

        mSystemManager->EntitySignatureChanged(entity, signature);
    }

    /// Remove a component to Entity
    /// @code
    /// gCoordinator.RemoveComponent<{YourComponent}>({YourEntity});
    /// @endcode
    template <typename T>
    void RemoveComponent(Entity entity)
    {
        mComponentManager->RemoveComponent<T>(entity);

        auto signature = mEntityManager->GetSignature(entity);
        signature.set(mComponentManager->GetComponentType<T>(), false);
        mEntityManager->SetSignature(entity, signature);

        mSystemManager->EntitySignatureChanged(entity, signature);
    }

    /// Get component from Entity
    /// @code
    /// gCoordinator.GetComponent<{YourComponent}>({YourEntity});
    /// @endcode
    template <typename T>
    T &GetComponent(Entity entity)
    {
        return mComponentManager->GetComponent<T>(entity);
    }

    /// Get component type, return in uint8_t;
    /// Needed to create signature
    /// @code
    /// gCoordinator.GetComponentType<{YourComponent}>();
    /// @endcode
    template <typename T>
    ComponentType GetComponentType()
    {
        return mComponentManager->GetComponentType<T>();
    }

    std::unordered_map<const char *, ComponentType> getComponentsTypes();

    // System methods

    /// Register a system before its usage
    /// @code
    /// gCoordinator.RegisterSystem<{YourSystem}>();
    /// @endcode
    template <typename T>
    std::shared_ptr<T> RegisterSystem()
    {
        auto system = mSystemManager->RegisterSystem<T>();
        mSystems.push_back(system);
        return system;
    }

    /// Set a signature on your System to get the good list of entity.
    /// In this exemple, the system will pass on all entity wich have {YourComponent}
    /// @code
    /// Signature signature;
    /// signature.set(gCoordinator.GetComponentType<{YourComponent}>());
    /// signature.set(gCoordinator.GetComponentType<Transform>());
    /// gCoordinator.SetSystemSignature<{YourSystem}>(signature);
    /// @endcode
    template <typename T>
    void SetSystemSignature(Signature signature)
    {
        mSystemManager->SetSignature<T>(signature);
    }

    void Update(float dt);

    // Event methods

    /// Add a event listener who will treat the Event that as been send
    /// The eventId is a uint32_t
    /// @code
    /// gCoordinator.AddEventListener({YourEventId}, {YourFunction});
    /// @endcode
    void AddEventListener(EventId eventId, std::function<void(Event &)> const &listener);

    /// Send an Event (with data)
    /// @code
    /// Event event({YourEventId});
    /// event.SetParam({ParamID}, {YourData});
    /// gCoordinator.SendEvent(event);
    /// @endcode
    void SendEvent(Event &event);

    /// Send an EventId
    /// @code
    /// gCoordinator.SendEvent({YourEventId});
    /// @endcode
    void SendEvent(EventId eventId);

    // Camera
    void setCamera(std::uint16_t camera);
    void addCamera(Entity camera);
    void switchCamera();
    Camera &getCamera();
    std::vector<Entity> &getCameras();

private:
    std::string name;
    std::unique_ptr<ComponentManager> mComponentManager;
    std::unique_ptr<EntityManager> mEntityManager;
    std::unique_ptr<EventManager> mEventManager;
    std::unique_ptr<SystemManager> mSystemManager;
    std::vector<std::shared_ptr<System>> mSystems;
    std::vector<Entity> mCamera;
    std::uint16_t mCurrentCamera;
};
