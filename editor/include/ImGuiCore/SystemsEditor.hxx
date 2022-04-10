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
    SystemsEditor(const pivot::ecs::systems::Index &index, pivot::ecs::SceneManager &sceneManager)
        : m_index(index), m_sceneManager(sceneManager){};

    void create();

private:
    void displaySystem();

    void createPopUp();

    const pivot::ecs::systems::Index &m_index;
    pivot::ecs::SceneManager &m_sceneManager;
};
