#pragma once

#include "pivot/ecs/Core/EcsException.hxx"
#include "pivot/ecs/Core/Scene.hxx"
#include <string>

using LevelId = std::uint16_t;
const LevelId MAX_LEVELS = UINT16_MAX; // 65535

class SceneManager {

public:
    void Init();

    LevelId registerLevel(std::string name);
    LevelId registerLevel();
    void    unregisterLevel(LevelId toDelete);

    LevelId getCurrentLevelId();
    void    setCurrentLevelId(LevelId newLevel);

    Scene &getCurrentLevel();
    Scene &getLevelById(LevelId idToGet);
    Scene &operator[](LevelId id);

    std::size_t getLivingScene();
private:
    std::unordered_map<LevelId, Scene> _levels{};
    LevelId _currentActiveLevel;
};
