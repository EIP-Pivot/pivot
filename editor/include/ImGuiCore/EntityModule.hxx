#pragma once

#include <imgui.h>

#include <pivot/ecs/Core/SceneManager.hxx>

#include <pivot/ecs/Core/types.hxx>

#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/Component/manager.hxx>

class EntityModule
{
public:
    EntityModule(): entitySelected(-1), _hasSelected(false), currentScene(-1){};
    void create();
    Entity addEntity();
    Entity addEntity(std::string name);
    void removeEntity();
    Entity getEntitySelected();
    bool hasSelected();

private:
    void createPopUp();
    Entity entitySelected;
    bool _hasSelected;
    LevelId currentScene;
};