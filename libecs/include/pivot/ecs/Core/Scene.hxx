#pragma once

#include "pivot/ecs/Core/Component/index.hxx"
#include "pivot/ecs/Core/Component/manager.hxx"
#include "pivot/ecs/Core/EntityManager.hxx"
#include "pivot/ecs/Core/EventManager.hxx"
#include "pivot/ecs/Core/SystemManager.hxx"
#include "pivot/ecs/Core/types.hxx"
#include <memory>

#include "pivot/ecs/Components/Camera.hxx"
#include "pivot/ecs/Components/Tag.hxx"

/// @class IScene
///
/// @brief Scene interface for transitional event
class IScene
{
protected:
    /// Called when scene is create
    virtual void OnCreate(){};
    /// Called every frame
    virtual void OnTick(){};
    /// Called when scene is set to pause
    virtual void OnPause(){};
    /// Called when scene is resume
    virtual void OnResume(){};
    /// Called when scene is delete
    virtual void OnStop(){};
};

/// @class Scene
///
/// @brief Coordinates the manager together
class Scene : public IScene
{
public:
    /// Default constructor, if no name specified it will generate a name like Scene 1
    Scene(std::string sceneName = "Scene");

    /// Get scene name
    std::string getName();

    /// Set scene name
    void setName(std::string newName) { name = newName; }

    // Entity methods

    /// Create a new entity
    /// @return Entity New entity
    Entity CreateEntity();

    /// Create e named entity
    Entity CreateEntity(std::string newName);

    /// Get entity list
    std::unordered_map<Entity, Signature> getEntities();

    /// @param[in] entity  Entity to remove.
    void DestroyEntity(Entity entity);

    /// Check if entity get a component
    template <typename T>
    [[deprecated]] bool hasComponent(Entity entity)
    {
        Signature entitySignature = mEntityManager->GetSignature(entity);
        auto id = GetComponentType<T>();
        return entitySignature.test(id);
    }
    /// Get signature of an entity
    Signature getSignature(Entity entity);

    /// Get name of an entity
    std::string getEntityName(Entity entity);

    /// Get the number of entity in the scene
    uint32_t getLivingEntityCount();

    // Component methods

    /// Register a new component before its usage
    /// @code
    /// gCoordinator.RegisterComponent<{YourComponent}>();
    /// @endcode
    template <typename T>
    [[deprecated]] void RegisterComponent()
    {
        auto name = mComponentIndex.getComponentNameByType<T>();
        auto description = mComponentIndex.getDescription(name.value());
        mComponentManager->RegisterComponent(description.value());
    }

    /// Check if the component is register in the scene
    template <typename T>
    [[deprecated]] bool isRegister()
    {
        auto name = mComponentIndex.getComponentNameByType<T>();
        return mComponentManager->GetComponentId(name.value()).has_value();
    }

    /// Add a component to Entity
    /// @code
    /// gCoordinator.AddComponent<{YourComponent}>({YourEntity});
    /// @endcode
    template <typename T>
    [[deprecated]] void AddComponent(Entity entity, T component)
    {
        auto name = mComponentIndex.getComponentNameByType<T>();
        auto id = mComponentManager->GetComponentId(name.value());
        mComponentManager->AddComponent(entity, std::any(component), id.value());

        auto signature = mEntityManager->GetSignature(entity);
        signature.set(id.value(), true);
        mEntityManager->SetSignature(entity, signature);

        mSystemManager->EntitySignatureChanged(entity, signature);
    }

    /// Remove a component to Entity
    /// @code
    /// gCoordinator.RemoveComponent<{YourComponent}>({YourEntity});
    /// @endcode
    template <typename T>
    [[deprecated]] void RemoveComponent(Entity entity)
    {
        auto name = mComponentIndex.getComponentNameByType<T>();
        auto id = mComponentManager->GetComponentId(name.value());
        mComponentManager->RemoveComponent(entity, id.value());

        auto signature = mEntityManager->GetSignature(entity);
        signature.set(id.value(), false);
        mEntityManager->SetSignature(entity, signature);

        mSystemManager->EntitySignatureChanged(entity, signature);
    }

    /// Get component from Entity
    /// @code
    /// gCoordinator.GetComponent<{YourComponent}>({YourEntity});
    /// @endcode
    template <typename T>
    [[deprecated]] T &GetComponent(Entity entity)
    {
        auto id = GetComponentType<T>();
        auto component = mComponentManager->GetComponentRef(entity, id);
        return std::any_cast<std::reference_wrapper<T>>(component).get();
    }

    /// Get component type, return in uint8_t;
    /// Needed to create signature
    /// @code
    /// gCoordinator.GetComponentType<{YourComponent}>();
    /// @endcode
    template <typename T>
    [[deprecated]] pivot::ecs::component::Manager::ComponentId GetComponentType()
    {
        auto name = mComponentIndex.getComponentNameByType<T>();
        auto id = mComponentManager->GetComponentId(name.value());
        return id.value();
    }

    /// Get the component manager
    pivot::ecs::component::Manager &getComponentManager();

    /// Get the component manager (const)
    const pivot::ecs::component::Manager &getComponentManager() const;

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
        for (Entity entity = 0; entity < getLivingEntityCount(); entity++) {
            if ((getSignature(entity) & signature) == signature) mSystemManager->setEntityToSystem<T>(entity);
        }
    }

    /// Check if a system is register in a scene
    template <typename T>
    bool hasSystem()
    {
        return mSystemManager->hasSystem<T>();
    }

    /// Update registered systems
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
    /// Set camera to use
    void setCamera(std::uint16_t camera);
    /// Add camera to scene camera list
    void addCamera(Entity camera);
    /// Switch camera
    void switchCamera();
    /// Get current camera
    Camera &getCamera();
    /// Get camera list
    std::vector<Entity> &getCameras();

    /// Save Scene in json file
    void save();

private:
    std::string name;
    std::unique_ptr<pivot::ecs::component::Manager> mComponentManager;
    std::unique_ptr<EntityManager> mEntityManager;
    std::unique_ptr<EventManager> mEventManager;
    std::unique_ptr<SystemManager> mSystemManager;
    std::vector<std::shared_ptr<System>> mSystems;
    std::vector<Entity> mCamera;
    std::uint16_t mCurrentCamera;
    pivot::ecs::component::Manager::ComponentId mTagId;
    pivot::ecs::component::Index mComponentIndex;
};
