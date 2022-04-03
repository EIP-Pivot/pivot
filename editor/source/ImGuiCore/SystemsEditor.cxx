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
    auto &sm = gSceneManager.getCurrentLevel().getSystemManager();
    if (ImGui::BeginPopup("AddSystem")) {
        for (const auto &[name, description]: GlobalIndex::getSingleton()) {
            if (!sm.hasSystem(name))
                if (ImGui::MenuItem(name.c_str())) sm.useSystem(name);
        }
        ImGui::EndPopup();
    }
}

void SystemsEditor::displaySystem()
{
    auto &sm = gSceneManager.getCurrentLevel().getSystemManager();
    for (const auto &[name, description]: sm) {
        if (ImGui::TreeNode(name.c_str())) ImGui::TreePop();
    }
}
