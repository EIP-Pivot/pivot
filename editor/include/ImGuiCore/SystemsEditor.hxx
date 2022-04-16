#pragma once

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>

#include "Systems/PhysicsSystem.hxx"

#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>

class SystemsEditor
{
public:
    SystemsEditor(const pivot::ecs::systems::Index &index, pivot::ecs::CurrentScene scene)
        : m_index(index), m_scene(scene){};

    void create();

private:
    void displaySystem();

    void createPopUp();

    const pivot::ecs::systems::Index &m_index;
    pivot::ecs::CurrentScene m_scene;
};
