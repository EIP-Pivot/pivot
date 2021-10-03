#pragma once

#include "pivot/ecs/Core/EcsException.hxx"
#include "pivot/ecs/Core/Coordinator.hxx"
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

class LevelManager {

public:
    void Init();

    LevelId registerLevel();
    void    unregisterLevel(LevelId toDelete);

    LevelId getCurrentLevelId();
    void    setCurrentLevelId(LevelId newLevel);

    Coordinator &getCurrentLevel();

private:
    std::unordered_map<LevelId, Coordinator> _levels{};
    LevelId _currentActiveLevel;
};
