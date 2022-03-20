#pragma once

#include "pivot/ecs/Core/Component/index.hxx"
#include "pivot/ecs/Core/Component/manager.hxx"

#include "pivot/ecs/Core/Systems/index.hxx"
#include "pivot/ecs/Core/Systems/manager.hxx"

#include "pivot/ecs/Core/Event/manager.hxx"

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

    /// Get signature of an entity
    Signature getSignature(Entity entity);

    /// Get name of an entity
    std::string getEntityName(Entity entity);

    /// Get the number of entity in the scene
    uint32_t getLivingEntityCount();

    /// Get the component manager
    pivot::ecs::component::Manager &getComponentManager();

    /// Get the component manager (const)
    const pivot::ecs::component::Manager &getComponentManager() const;

    // System methods

    /// Get the system manager
    pivot::ecs::systems::Manager &getSystemManager();

    /// Get the system manager (const)
    const pivot::ecs::systems::Manager &getSystemManager() const;

    // Event methods

    /// Get the event manager
    pivot::ecs::event::Manager &getEventManager();

    /// Get the event manager (const)
    const pivot::ecs::event::Manager &getEventManager() const;

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

private:
    std::string name;
    pivot::ecs::component::Manager mComponentManager;
    EntityManager mEntityManager;
    pivot::ecs::systems::Manager mSystemManager;
    pivot::ecs::event::Manager mEventManager;
    std::vector<std::shared_ptr<System>> mSystems;
    std::vector<Entity> mCamera;
    std::uint16_t mCurrentCamera;
    pivot::ecs::component::Manager::ComponentId mTagId;
    pivot::ecs::component::Index mComponentIndex;
};
