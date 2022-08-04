#include "ImGuiCore/ImGuiManager.hxx"

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

void ImGuiManager::newFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    menuBar();
    dockSpace();
}

void ImGuiManager::dockSpace()
{

    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGuiWindowFlags host_window_flags = 0;
        host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
        host_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            host_window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Window", nullptr, host_window_flags);
        {
            ImGui::PopStyleVar(3);

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

            ImGuiID dockspace_id = ImGui::GetID("DockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags, nullptr);

            ImGui::PopStyleColor();
        }
    }
    ImGui::End();
}

void ImGuiManager::menuBar()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{10.0f, 10.0f});
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            handleFile<FileAction::Save>(
                "Save Scene", "Scene correctly saved.", "Failed to save the scene, please check the log.",
                {{"Scene", "json"}},
                [this](const std::filesystem::path &path) {
                    m_engine.saveScene(m_sceneManager.getCurrentSceneId(), path);
                    return true;
                },
                "CTRL+S");
            handleFile<FileAction::Open>("Load Scene", "Scene loaded succefully !",
                                         "Scene loading failed, please check the log.", {{"Scene", "json"}},
                                         [this](const std::filesystem::path &path) {
                                             m_engine.loadScene(path);
                                             return true;
                                         });
            handleFile<FileAction::Open>("Load Script", "Script loaded succefully !",
                                         "Script loading failed, please look at the logs.",
                                         {{"PivotScript", "pivotscript"}}, [this](const std::filesystem::path &path) {
                                             m_engine.loadScript(path);
                                             return true;
                                         });
            handleFile<FileAction::Open>(
                "Load asset", "Asset loaded succefully !", "Asset loading failed, please look at the logs.",
                {{"Model", "gltf,obj"}, {"Textures", "jpg,png,ktx"}}, [this](const std::filesystem::path &path) {
                    m_engine.loadAsset(path);
                    return true;
                });
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    ImGui::PopStyleVar();
}

void ImGuiManager::render() { ImGui::Render(); }
