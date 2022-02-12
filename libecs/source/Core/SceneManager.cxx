#include "pivot/ecs/Core/SceneManager.hxx"

LevelId SceneManager::registerLevel(std::string name)
{
    _levels.insert({_levels.size(), std::make_unique<Scene>(name)});
    return (LevelId(_levels.size() - 1));
}

LevelId SceneManager::registerLevel()
{
    _levels.insert({_levels.size(), std::make_unique<Scene>("Scene " + std::to_string(LevelId(_levels.size())))});
    return (LevelId(_levels.size() - 1));
}

void SceneManager::unregisterLevel(LevelId toDelete)
{
    if (!_levels.contains(toDelete))
        throw EcsException("Scene with id _" + std::to_string(toDelete) +
                           "_ doesn't exist. Register it before trying to delete it.");

    _levels.erase(toDelete);
}

LevelId SceneManager::getCurrentLevelId() { return _currentActiveLevel; }

void SceneManager::setCurrentLevelId(LevelId newScene)
{
    if (newScene >= _levels.size())
        throw EcsException("Scene with id _" + std::to_string(newScene) + "_ hasn't been registered.");

    _currentActiveLevel = newScene;
}

Scene &SceneManager::getCurrentLevel()
{
    if (_currentActiveLevel == -1)
        throw EcsException("There is no current level. Register a level before trying to access its Scene.");
    
    return *this->_levels[_currentActiveLevel].get();
}

Scene &SceneManager::getLevelById(LevelId idToGet)
{
    if (!_levels.contains(idToGet))
        throw EcsException("Level with id _" + std::to_string(idToGet) + "_ is not registered.");
    
    return *this->_levels[idToGet].get();
}

Scene &SceneManager::operator[](LevelId id) { return getLevelById(id); }

std::size_t SceneManager::getLivingScene() { return _levels.size(); }
