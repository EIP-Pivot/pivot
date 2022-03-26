#pragma once

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

class ImGuiManager
{
public:
    void newFrame();
    void render();
};
