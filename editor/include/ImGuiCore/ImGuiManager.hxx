#pragma once

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>

class ImGuiManager
{
public:
    ImGuiManager(const pivot::ecs::SceneManager &sceneManager, const pivot::ecs::systems::Index &index_S, const pivot::ecs::component::Index &index_C):
        m_sceneManager(sceneManager), Sindex(index_S), Cindex(index_C){};
    void newFrame();
    void render();
private:
    const pivot::ecs::SceneManager &m_sceneManager;
    const pivot::ecs::systems::Index &Sindex;
    const pivot::ecs::component::Index &Cindex;
};
