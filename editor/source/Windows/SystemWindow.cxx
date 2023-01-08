#include "Windows/SystemWindow.hxx"

#include <imgui.h>

#include "ImGuiCore/CustomWidget.hxx"
#include "ImGuiCore/ImGuiTheme.hxx"

using namespace pivot::editor;

void SystemWindow::render()
{
    PROFILE_FUNCTION();
    ImGui::Begin(" Systems ", &m_open);
    ImGuiTheme::setDefaultFramePadding();
    displaySystem();
    if (CustomWidget::ButtonCenteredOnLine("Add System")) { ImGui::OpenPopup("AddSystem"); }
    createPopUp();
    ImGuiTheme::unsetDefaultFramePadding();
    ImGui::End();
}

void SystemWindow::createPopUp()
{
    PROFILE_FUNCTION();
    auto &sm = m_manager.getCurrentScene()->getSystemManager();
    if (ImGui::BeginPopup("AddSystem")) {
        for (const auto &[name, description]: m_manager.getSystemIndex()) {
            if (!sm.hasSystem(name))
                if (ImGui::MenuItem(name.c_str()))
                    m_manager.getCurrentScene()->registerSystem(description,
                                                                std::optional(std::ref(m_manager.getComponentIndex())));
        }
        ImGui::EndPopup();
    }
}

void SystemWindow::displaySystem()
{
    PROFILE_FUNCTION();
    auto &sm = m_manager.getCurrentScene()->getSystemManager();
    for (const auto &[name, description]: sm) {
        if (ImGui::TreeNode(name.c_str())) ImGui::TreePop();
    }
}
