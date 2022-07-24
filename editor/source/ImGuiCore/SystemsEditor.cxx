#include <Logger.hpp>

#include "ImGuiCore/SystemsEditor.hxx"

void SystemsEditor::create()
{
    ImGui::Begin("Systems");
    displaySystem();
    if (ImGui::Button("Add System")) { ImGui::OpenPopup("AddSystem"); }
    createPopUp();
    ImGui::End();
}

void SystemsEditor::createPopUp()
{
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
    auto &sm = m_scene->getSystemManager();
    for (const auto &[name, description]: sm) {
        if (ImGui::TreeNode(name.c_str())) ImGui::TreePop();
    }
}
