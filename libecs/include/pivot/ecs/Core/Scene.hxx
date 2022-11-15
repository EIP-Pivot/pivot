#pragma once

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include <pivot/pivot.hxx>

#include <pivot/ecs/Core/Data/value_serialization.hxx>

#include "pivot/ecs/Core/Component/index.hxx"
#include "pivot/ecs/Core/Component/manager.hxx"

#include "pivot/ecs/Core/Systems/index.hxx"
#include "pivot/ecs/Core/Systems/manager.hxx"

#include "pivot/ecs/Core/Event/manager.hxx"

#include "pivot/ecs/Core/EntityManager.hxx"
#include "pivot/ecs/Core/types.hxx"
#include <memory>

#include "pivot/ecs/Components/Tag.hxx"

namespace pivot::ecs
{

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
    const std::string &getName() const;

    /// Set scene name
    void setName(std::string newName) { name = newName; }

    // Entity methods

    /// Create a new entity
    /// @return Entity New entity
    Entity CreateEntity();

    /// Create e named entity
    Entity CreateEntity(std::string newName);

    /// Get entity list
    std::unordered_map<Entity, Signature> getEntities() const;

    /// @param[in] entity  Entity to remove.
    void DestroyEntity(Entity entity);

    /// Get signature of an entity
    Signature getSignature(Entity entity);

    /// Get name of an entity
    std::string getEntityName(Entity entity);

    /// Get the id of an entity by its name
    std::optional<Entity> getEntityID(const std::string &name);

    /// Get the number of entity in the scene
    uint32_t getLivingEntityCount();

    /// Get the component manager
    pivot::ecs::component::Manager &getComponentManager();

    /// Get the component manager (const)
    const pivot::ecs::component::Manager &getComponentManager() const;

    // System methods

    /// Get the system manager (const)
    const pivot::ecs::systems::Manager &getSystemManager() const;

    /// Register a system in the scene
    ///
    /// This optionally automatically registers needed components, by taking them from cIndex
    void registerSystem(const systems::Description &description,
                        pivot::OptionalRef<const component::Index> cIndex = std::nullopt);

    /// Get the Entity manager
    EntityManager &getEntityManager();

    /// Get the Entity manager (const)
    const EntityManager &getEntityManager() const;

    // Event methods

    /// Get the event manager
    pivot::ecs::event::Manager &getEventManager();

    /// Get the event manager (const)
    const pivot::ecs::event::Manager &getEventManager() const;

    /// Function used to retrieve the real path of an asset if possible
    using AssetTranslator = std::function<std::optional<std::string>(const std::string &)>;
    /// Function used to retrieve the real path of an external ressource
    using ScriptTranslator = std::function<std::optional<std::string>(const std::string &)>;
    /// Save scene in json file
    void save(const std::filesystem::path &path, std::optional<AssetTranslator> assetTranslator = std::nullopt,
              std::optional<ScriptTranslator> scriptTranslator = std::nullopt) const;

    // Load
    /// Load a scene from JSON object
    static std::unique_ptr<Scene> load(const nlohmann::json &obj, const pivot::ecs::component::Index &cIndex,
                                       const pivot::ecs::systems::Index &sIndex);

private:
    std::string name;
    pivot::ecs::component::Manager mComponentManager;
    EntityManager mEntityManager;
    pivot::ecs::systems::Manager mSystemManager;
    pivot::ecs::event::Manager mEventManager;
    pivot::ecs::component::Manager::ComponentId mTagId;
};

}    // namespace pivot::ecs
