#include "pivot/ecs/Core/SceneManager.hxx"

LevelId SceneManager::registerLevel(std::string name)
{
    _levels.push_back(std::make_unique<Scene>(name));
    return (LevelId(_levels.size() - 1));
}

LevelId SceneManager::registerLevel()
{
    _levels.push_back(std::make_unique<Scene>("Scene " + std::to_string(LevelId(_levels.size()))));
    return (LevelId(_levels.size() - 1));
}

void SceneManager::unregisterLevel(LevelId toDelete)
{
    if (toDelete >= _levels.size() || !_levels[toDelete].has_value())
        throw EcsException("Scene with id _" + std::to_string(toDelete) +
                           "_ doesn't exist. Register it before trying to delete it.");

    _levels[toDelete] = std::nullopt;
}

LevelId SceneManager::getCurrentLevelId() { return _currentActiveLevel.value(); }

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

    return *this->_levels.at(_currentActiveLevel.value()).value();
}

Scene &SceneManager::getLevelById(LevelId idToGet)
{
    if (idToGet >= _levels.size() || !_levels[idToGet].has_value())
        throw EcsException("Level with id _" + std::to_string(idToGet) + "_ is not registered.");

    return *this->_levels.at(idToGet).value();
}

Scene &SceneManager::operator[](LevelId id) { return getLevelById(id); }

std::size_t SceneManager::getLivingScene() { return _levels.size(); }
