#include "ImGuiCore/ImGuiManager.hxx"

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

void ImGuiManager::newFrame(pivot::Engine &engine)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{10.0f, 10.0f});
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            handleFile<FileAction::Save>(
                "Save Scene", "Scene correctly saved.", "Failed to save the scene, please check the log.",
                {{"Scene", "json"}},
                [&engine, this](const std::filesystem::path &path) {
                    engine.saveScene(m_sceneManager.getCurrentSceneId(), path);
                    return true;
                },
                "CTRL+S");
            handleFile<FileAction::Open>("Load Scene", "Scene loaded succefully !",
                                         "Scene loading failed, please check the log.", {{"Scene", "json"}},
                                         [&engine](const std::filesystem::path &path) {
                                             engine.loadScene(path);
                                             return true;
                                         });
            handleFile<FileAction::Open>(
                "Load Script", "Script loaded succefully !", "Script loading failed, please look at the logs.",
                {{"PivotScript", "pivotscript"}}, [&engine](const std::filesystem::path &path) {
                    engine.loadScript(path);
                    return true;
                });
            handleFile<FileAction::Open>(
                "Load asset", "Asset loaded succefully !", "Asset loading failed, please look at the logs.",
                {{"Model", "gltf,obj"}, {"Textures", "jpg,png,ktx"}}, [&engine](const std::filesystem::path &path) {
                    engine.loadAsset(path);
                    return true;
                });
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    ImGui::PopStyleVar();
}

void ImGuiManager::render() { ImGui::Render(); }
