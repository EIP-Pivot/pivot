#pragma once

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <pivot/ecs/Core/SceneManager.hxx>

class ImGuiManager
{
public:
    ImGuiManager(const pivot::ecs::SceneManager &sceneManager): m_sceneManager(sceneManager){};
    void newFrame();
    void render();
private:
    const pivot::ecs::SceneManager &m_sceneManager;
};
