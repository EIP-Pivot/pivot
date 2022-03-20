#pragma once

#include <string>

#include "pivot/ecs/Core/EcsException.hxx"
#include "pivot/ecs/Core/Scene.hxx"

using LevelId = std::uint16_t;
const LevelId MAX_LEVELS = std::numeric_limits<LevelId>::max();    // 65535

/// @class SceneManager
///
/// @brief Scene manager is the main part of the ecs, a global is set for using it everywhere: gSceneManager (
/// gSceneManager will be remove)
class SceneManager
{
public:
    SceneManager(): _currentActiveLevel(std::nullopt){};

    /// Create new scene with scene name
    LevelId registerLevel(std::string name);
    /// Create new scene with default name (ex: Scene 1)
    LevelId registerLevel();

    /// Remove scene
    void unregisterLevel(LevelId toDelete);

    /// Get current scene id
    LevelId getCurrentLevelId();
    /// Set current scene id
    void setCurrentLevelId(LevelId newLevel);

    /// Get current scene object
    Scene &getCurrentLevel();
    /// Get scene object with an id
    Scene &getLevelById(LevelId idToGet);
    /// Operator overload [] to get scene with id
    Scene &operator[](LevelId id);

    /// Get living scene
    std::size_t getLivingScene();

private:
    std::vector<std::optional<std::unique_ptr<Scene>>> _levels{};
    std::optional<LevelId> _currentActiveLevel;
};
