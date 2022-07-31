#pragma once

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>

#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>

class SystemsEditor
{
public:
    SystemsEditor(const pivot::ecs::systems::Index &systemIndex, const pivot::ecs::component::Index &componentIndex,
                  pivot::ecs::CurrentScene scene)
        : m_systemIndex(systemIndex), m_componentIndex(componentIndex), m_scene(scene){};

    void create();

private:
    void displaySystem();

    void createPopUp();

    const pivot::ecs::systems::Index &m_systemIndex;
    const pivot::ecs::component::Index &m_componentIndex;
    pivot::ecs::CurrentScene m_scene;
};
