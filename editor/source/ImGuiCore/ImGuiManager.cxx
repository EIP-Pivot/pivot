#include "ImGuiCore/ImGuiManager.hxx"

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui_internal.h>

void ImGuiManager::newFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    // If menuBar() return true, that mean that it is no longer safe to continue rendering, nor to use the TextureIds.
    if (menuBar()) {
        // reset texture Ids and start a new frame
        reset();
        return newFrame();
    }
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

        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode |
                                             ImGuiDockNodeFlags_NoDockingInCentralNode |
                                             ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton;
        ImGuiWindowFlags host_window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                             ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                             ImGuiWindowFlags_NoNavFocus;
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
            ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2{0.f, 0.f});
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 5.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags, nullptr);
            ImGui::PopStyleVar(3);
            if (ImGuiDockNode *node = ImGui::DockBuilderGetCentralNode(dockspace_id)) {
                m_centerDockId = node->ID;
                node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
            }
            ImGui::PopStyleColor();
        }
    }
    ImGui::End();
}

bool ImGuiManager::menuBar()
{
    MenuBarAction menuAction = MenuBarAction::None;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{10.0f, 10.0f});
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 5.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save Scene", "CTRL+S")) menuAction = MenuBarAction::SaveScene;
            if (ImGui::MenuItem("Load Scene")) menuAction = MenuBarAction::LoadScene;
            if (ImGui::MenuItem("Load Script")) menuAction = MenuBarAction::LoadScript;
            if (ImGui::MenuItem("Load asset")) menuAction = MenuBarAction::LoadAsset;

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    ImGui::PopStyleVar(3);

    switch (menuAction) {
        case MenuBarAction::None: return false;
        case MenuBarAction::SaveScene:
            handleFile<FileAction::Save>("Save Scene", "Scene correctly saved.",
                                         "Failed to save the scene, please check the log.", {{"Scene", "json"}},
                                         [this](const std::filesystem::path &path) {
                                             m_engine.saveScene(m_sceneManager.getCurrentSceneId(), path);
                                             return true;
                                         });
            break;
        case MenuBarAction::LoadScene:
            handleFile<FileAction::Open>("Load Scene", "Scene loaded succefully !",
                                         "Scene loading failed, please check the log.", {{"Scene", "json"}},
                                         [this](const std::filesystem::path &path) {
                                             m_engine.loadScene(path);
                                             return true;
                                         });
            break;
        case MenuBarAction::LoadScript:
            handleFile<FileAction::Open>("Load Script", "Script loaded succefully !",
                                         "Script loading failed, please look at the logs.",
                                         {{"Pivot script", "pivotscript"}}, [this](const std::filesystem::path &path) {
                                             m_engine.loadScript(path);
                                             return true;
                                         });
            break;
        case MenuBarAction::LoadAsset:
            handleFile<FileAction::Open>(
                "Load asset", "Asset loaded succefully !", "Asset loading failed, please look at the logs.",
                {{"Model", "gltf,obj"}, {"Textures", "jpg,png,ktx"}}, [this](const std::filesystem::path &path) {
                    m_engine.loadAsset(path);
                    return true;
                });
            break;
    }
    return true;
}

void ImGuiManager::render()
{
    ImGui::EndFrame();
    ImGui::Render();
    // ImGui::UpdatePlatformWindows();
}
ImGuiID ImGuiManager::getCenterDockId() { return m_centerDockId; }
