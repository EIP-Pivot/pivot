#pragma once

#include <string>

#include "pivot/ecs/Core/EcsException.hxx"
#include "pivot/ecs/Core/Scene.hxx"

namespace pivot::ecs
{
/// @class SceneManager
///
/// @brief Scene manager is the main part of the ecs, a global is set for using it everywhere: gSceneManager (
/// gSceneManager will be remove)
class SceneManager
{
public:
    SceneManager(): m_currentActiveScene(std::nullopt){};

    /// Type used to store the scenes
    using scene_storage = std::vector<std::optional<std::unique_ptr<Scene>>>;
    /// Id of a scene
    using SceneId = scene_storage::size_type;

    /// Create new scene with scene name
    SceneId registerScene(std::string name);
    /// Create new scene with default name (ex: Scene 1)
    SceneId registerScene();

    /// Remove scene
    void unregisterScene(SceneId toDelete);

    /// Get current scene id
    SceneId getCurrentSceneId() const;
    /// Set current scene id
    void setCurrentSceneId(SceneId newLevel);

    /// Get the id of a scene by its name, or nullopt if the scene does not exist
    std::optional<SceneId> getSceneId(const std::string &sceneName) const;

    /// Get current scene object
    Scene &getCurrentScene();
    /// Get current scene object (const)
    const Scene &getCurrentScene() const;
    /// Get scene object with an id
    Scene &getSceneById(SceneId idToGet);
    /// Operator overload [] to get scene with id
    Scene &operator[](SceneId id);
    /// Get scene object with an id
    const Scene &getSceneById(SceneId idToGet) const;
    /// Operator overload [] to get scene with id
    const Scene &operator[](SceneId id) const;

    /// Get living scene
    std::size_t getLivingScene() const;

private:
    scene_storage m_scenes{};
    std::map<std::string, SceneId> m_sceneNameToLevel;
    std::optional<SceneId> m_currentActiveScene;
};

/// Wrapper for the scene currently selected
class CurrentScene
{
public:
    /// Wrap current scene of a scene manager
    CurrentScene(SceneManager &sm): m_sceneManager(sm) {}

    /// Access current scene
    Scene *operator->() { return &m_sceneManager.getCurrentScene(); }
    /// Access current scene
    const Scene *operator->() const { return &m_sceneManager.getCurrentScene(); }
    /// Access current scene
    Scene &operator*() { return m_sceneManager.getCurrentScene(); }
    /// Access current scene
    const Scene &operator*() const { return m_sceneManager.getCurrentScene(); }

    /// Get id of the current scene
    SceneManager::SceneId id() const { return m_sceneManager.getCurrentSceneId(); }

private:
    SceneManager &m_sceneManager;
};

}    // namespace pivot::ecs
