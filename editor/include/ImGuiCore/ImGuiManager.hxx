#pragma once

#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/engine.hxx>

//to delete
#include <imgui.h>

#include <ImGuizmo.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#include <unistd.h>
#include <sys/wait.h>
#include <iostream>

//

void LoadingIndicatorCircle(const char *label, const float size);

class ImGuiManager
{
public:
    ImGuiManager(const pivot::ecs::SceneManager &sceneManager): m_sceneManager(sceneManager){};
    void newFrame(pivot::Engine &engine);
    void saveScene(pivot::Engine &engine);
    void loadScene(pivot::Engine &engine);
    void loadAsset(pivot::Engine &engine);
    void loadScript(pivot::Engine &engine);
    void render();

private:
    const pivot::ecs::SceneManager &m_sceneManager;
};
