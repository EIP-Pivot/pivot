#pragma once

#include <imgui.h>

#include <pivot/ecs/Core/SceneManager.hxx>

#include <pivot/ecs/Core/types.hxx>

#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/Component/manager.hxx>

class EntityModule
{
public:
    EntityModule(pivot::ecs::CurrentScene scene)
        : m_scene(scene), entitySelected(-1), _hasSelected(false), currentScene(-1){};
    void create();
    Entity addEntity();
    Entity addEntity(std::string name);
    void removeEntity();
    Entity getEntitySelected();
    bool hasSelected();

private:
    void createPopUp();

    pivot::ecs::CurrentScene m_scene;

    Entity entitySelected;
    bool _hasSelected;
    pivot::ecs::SceneManager::SceneId currentScene;
};
