#pragma once

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>

#include "Systems/PhysicsSystem.hxx"

#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>

extern SceneManager gSceneManager;

class SystemsEditor
{
public:
    SystemsEditor(const pivot::ecs::systems::Index &index): m_index(index){};

    void create();

private:
    void displaySystem();

    void createPopUp();

    const pivot::ecs::systems::Index &m_index;
};
