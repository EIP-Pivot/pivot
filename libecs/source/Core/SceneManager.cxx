#include "pivot/ecs/Core/SceneManager.hxx"

void SceneManager::Init() {
    _levels.clear();
    _currentActiveLevel = -1;
}

LevelId SceneManager::registerLevel() {
    // if (_levels.size() >= MAX_LEVELS)
    //     throw EcsException("SceneManager is already at max capacity. Delete a scene before registering a new one.");

    _levels[_levels.size() + 1] = Scene();
    return (_levels.size() - 1);
}

void SceneManager::unregisterLevel(LevelId toDelete) {
    if (!_levels.contains(toDelete))
        throw EcsException("Scene with id _" + std::to_string(toDelete) + "_ doesn't exist. Register it before trying to delete it.");

    _levels.erase(toDelete);
}

LevelId SceneManager::getCurrentLevelId() {
    return _currentActiveLevel;
}

void SceneManager::setCurrentLevelId(LevelId newScene) {
    if (newScene >= _levels.size())
        throw EcsException("Scene with id _" + std::to_string(newScene) + "_ hasn't been registered.");

    _currentActiveLevel = newScene;
}

Scene &SceneManager::getCurrentLevel() {
    if (_currentActiveLevel == -1)
        throw EcsException("There is no current level. Register a level before trying to access its Scene.");
    
    return _levels[_currentActiveLevel];
}

Scene &SceneManager::getLevelById(LevelId idToGet) {
    if (!_levels.contains(idToGet))
        throw EcsException("Level with id _" + std::to_string(idToGet) + "_ is not registered.");
    
    return _levels[idToGet];
}

Scene &SceneManager::operator[](LevelId id) {
    return getLevelById(id);
}
