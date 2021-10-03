#include "pivot/ecs/Core/LevelManager.hxx"

void LevelManager::Init() {
    _levels.clear();
    _currentActiveLevel = -1;
}

LevelId LevelManager::registerLevel() {
    if (_levels.size() >= MAX_LEVELS)
        throw EcsException("LevelManager is already at max capacity. Delete a scene before registering a new one.");

    _levels[_levels.size() + 1] = Coordinator();
    return (_levels.size() - 1);
}

void LevelManager::unregisterLevel(LevelId toDelete) {
    if (toDelete >= _levels.size())
        throw EcsException("Scene with id _" + std::to_string(toDelete) + "_ doesn't exist. Register it before trying to delete it.");

    _levels.erase(toDelete);
}

LevelId LevelManager::getCurrentLevelId() {
    return _currentActiveLevel;
}

void LevelManager::setCurrentLevelId(LevelId newScene) {
    if (newScene >= _levels.size())
        throw EcsException("Scene with id _" + std::to_string(newScene) + "_ hasn't been registered.");

    _currentActiveLevel = newScene;
}

Coordinator &LevelManager::getCurrentLevel() {
    if (_currentActiveLevel == -1)
        throw EcsException("There is no current level. Register a level before trying to access its Coordinator.");
    
    return _levels[_currentActiveLevel];
}
