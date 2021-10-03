#pragma once

#include "pivot/ecs/Core/EcsException.hxx"
#include "pivot/ecs/Core/Scene.hxx"
#include <string>

using LevelId = std::uint16_t;
const LevelId MAX_LEVELS = UINT16_MAX; // 65535

class ILevel {
public:

protected:
    virtual void OnCreate() {};
    virtual void OnTick() {};
    virtual void OnPause() {};
    virtual void OnResume() {};
    virtual void OnStop() {};
};

class Level : public ILevel {
public:

protected:

};

class SceneManager {

public:
    void Init();

    LevelId registerLevel();
    void    unregisterLevel(LevelId toDelete);

    LevelId getCurrentLevelId();
    void    setCurrentLevelId(LevelId newLevel);

    Scene &getCurrentLevel();
    Scene &getLevelById(LevelId idToGet);
    Scene &operator[](LevelId id);


private:
    std::unordered_map<LevelId, Scene> _levels{};
    LevelId _currentActiveLevel;
};
