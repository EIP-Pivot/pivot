#include "ImGuiCore/SystemsEditor.hxx"
#include "ImGuiCore/CustomWidget.hxx"
#include "ImGuiCore/ImGuiTheme.hxx"

#include <imgui.h>

void SystemsEditor::create()
{
    PROFILE_FUNCTION();
    ImGui::Begin(" Systems ");
    ImGuiTheme::setDefaultFramePadding();
    displaySystem();
    if (CustomWidget::ButtonCenteredOnLine("Add System")) { ImGui::OpenPopup("AddSystem"); }
    createPopUp();
    ImGuiTheme::unsetDefaultFramePadding();
    ImGui::End();
}

void SystemsEditor::createPopUp()
{
    PROFILE_FUNCTION();
    auto &sm = m_scene->getSystemManager();
    if (ImGui::BeginPopup("AddSystem")) {
        for (const auto &[name, description]: m_systemIndex) {
            if (!sm.hasSystem(name))
                if (ImGui::MenuItem(name.c_str()))
                    m_scene->registerSystem(description, std::optional(std::ref(m_componentIndex)));
        }
        ImGui::EndPopup();
    }
}

void SystemsEditor::displaySystem()
{
    PROFILE_FUNCTION();
    auto &sm = m_scene->getSystemManager();
    for (const auto &[name, description]: sm) {
        if (ImGui::TreeNode(name.c_str())) ImGui::TreePop();
    }
}
