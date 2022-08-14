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
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 7.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags, nullptr);
            ImGui::PopStyleVar(3);
            if (ImGuiDockNode *node = ImGui::DockBuilderGetCentralNode(dockspace_id)) {
                m_centerDockId = node->ID;
                //                node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
            }
            ImGui::PopStyleColor();
        }
    }
    ImGui::End();
}

void ImGuiManager::render()
{
    ImGui::EndFrame();
    ImGui::Render();
    // ImGui::UpdatePlatformWindows();
}
ImGuiID ImGuiManager::getCenterDockId() { return m_centerDockId; }
