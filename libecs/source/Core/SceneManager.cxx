#include "pivot/ecs/Core/SceneManager.hxx"

namespace pivot::ecs
{
SceneManager::SceneId SceneManager::registerScene(std::string name)
{
    m_scenes.push_back(std::make_unique<Scene>(name));
    SceneId id = SceneId(m_scenes.size() - 1);
    m_sceneNameToLevel[name] = id;
    return (id);
}

SceneManager::SceneId SceneManager::registerScene()
{
    return this->registerScene("Scene " + std::to_string(m_scenes.size()));
}

void SceneManager::unregisterScene(SceneId toDelete)
{
    if (toDelete >= m_scenes.size() || !m_scenes[toDelete].has_value())
        throw EcsException("Scene with id _" + std::to_string(toDelete) +
                           "_ doesn't exist. Register it before trying to delete it.");

    m_scenes[toDelete] = std::nullopt;
}

SceneManager::SceneId SceneManager::getCurrentSceneId() const { return m_currentActiveScene.value(); }

void SceneManager::setCurrentSceneId(SceneId newScene)
{
    if (newScene >= m_scenes.size())
        throw EcsException("Scene with id _" + std::to_string(newScene) + "_ hasn't been registered.");

    m_currentActiveScene = newScene;
}

std::optional<SceneManager::SceneId> SceneManager::getSceneId(const std::string &sceneName) const
{
    auto id = m_sceneNameToLevel.find(sceneName);
    if (id == m_sceneNameToLevel.end()) {
        return std::nullopt;
    } else {
        return id->second;
    }
}

Scene &SceneManager::getCurrentScene()
{
    if (!m_currentActiveScene.has_value())
        throw EcsException("There is no current level. Register a level before trying to access its Scene.");

    return *this->m_scenes.at(m_currentActiveScene.value()).value();
}

const Scene &SceneManager::getCurrentScene() const
{
    if (!m_currentActiveScene.has_value())
        throw EcsException("There is no current level. Register a level before trying to access its Scene.");

    return *this->m_scenes.at(m_currentActiveScene.value()).value();
}

const Scene &SceneManager::getSceneById(SceneId idToGet) const
{
    if (idToGet >= m_scenes.size() || !m_scenes[idToGet].has_value())
        throw EcsException("Level with id _" + std::to_string(idToGet) + "_ is not registered.");

    return *this->m_scenes.at(idToGet).value();
}

const Scene &SceneManager::operator[](SceneId id) const { return getSceneById(id); }

Scene &SceneManager::getSceneById(SceneId id)
{
    return const_cast<Scene &>(const_cast<const SceneManager *>(this)->getSceneById(id));
}

Scene &SceneManager::operator[](SceneId id)
{
    return const_cast<Scene &>(const_cast<const SceneManager *>(this)->getSceneById(id));
}

std::size_t SceneManager::getLivingScene() const { return m_scenes.size(); }

}    // namespace pivot::ecs
