#pragma once

#include "pivot/ecs/Core/EcsException.hxx"
#include "pivot/ecs/Core/Scene.hxx"
#include <string>

using LevelId = std::int16_t;
const LevelId MAX_LEVELS = INT16_MAX; // 65535

/// @class SceneManager
///
/// @brief Scene manager is the main part of the ecs, a global is set for using it everywhere: gSceneManager (
/// gSceneManager will be remove)
class SceneManager {
public:
    SceneManager(): _currentActiveLevel(-1) {};

    /// Create new scene with scene name
    LevelId registerLevel(std::string name);
    /// Create new scene with default name (ex: Scene 1)
    LevelId registerLevel();

    /// Remove scene
    void    unregisterLevel(LevelId toDelete);

    /// Get current scene id
    LevelId getCurrentLevelId();
    /// Set current scene id
    void    setCurrentLevelId(LevelId newLevel);

    /// Get current scene object
    Scene &getCurrentLevel();
    /// Get scene object with an id
    Scene &getLevelById(LevelId idToGet);
    /// Operator overload [] to get scene with id
    Scene &operator[](LevelId id);

    /// Get living scene
    std::size_t getLivingScene();
private:
    std::unordered_map<LevelId, Scene> _levels{};
    LevelId _currentActiveLevel;
};
