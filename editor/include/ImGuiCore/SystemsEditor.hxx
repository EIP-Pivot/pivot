#pragma once

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>

#include "Systems/PhysicsSystem.hxx"

#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>

extern SceneManager gSceneManager;

using namespace pivot::ecs::systems;

class SystemsEditor
{
public:
    void create();

private:
    void displaySystem();

    void createPopUp();
};
