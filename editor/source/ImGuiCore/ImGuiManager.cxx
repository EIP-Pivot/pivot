#include "ImGuiCore/ImGuiManager.hxx"
#include <pivot/ecs/Core/SceneManager.hxx>

void ImGuiManager::newFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void ImGuiManager::render() { ImGui::Render(); }
