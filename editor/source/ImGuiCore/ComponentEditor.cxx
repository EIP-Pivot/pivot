#include "ImGuiCore/ComponentEditor.hxx"
#include "ImGuiCore/TypeTemplate/Template.hxx"
#include "ImGuiCore/TypeTemplate/createValue.hxx"
#include <magic_enum.hpp>
#include <misc/cpp/imgui_stdlib.h>

void ComponentEditor::create(Entity entity)
{
    currentEntity = entity;
    ImGui::Begin("Component editor");
    createPopUp();
    displayComponent();
    if (ImGui::Button("Add Component")) { ImGui::OpenPopup("AddComponent"); }
    ImGui::End();
}

void ComponentEditor::create()
{
    ImGui::Begin("Component editor");
    ImGui::Text("No entity selected.");
    ImGui::End();
}

void ComponentEditor::setVectorObject(LevelId scene) { sceneObject[scene] = ObjectVector(); }

std::unordered_map<LevelId, ObjectVector> &ComponentEditor::getVectorObject() { return sceneObject; }

ObjectVector ComponentEditor::getObject() { return sceneObject[m_sceneManager.getCurrentLevelId()]; }

void ComponentEditor::createPopUp()
{
    auto &cm = m_sceneManager.getCurrentLevel().getComponentManager();
    if (ImGui::BeginPopup("AddComponent")) {
        for (const auto &[name, description]: m_index) {
            if (cm.GetComponent(currentEntity, cm.GetComponentId(name).value()) == std::nullopt) {
                if (ImGui::MenuItem(name.c_str())) { addComponent(description); }
            }
        }
        ImGui::EndPopup();
    }
}

void ComponentEditor::displayComponent()
{
    auto &cm = m_sceneManager.getCurrentLevel().getComponentManager();
    for (ComponentRef ref: cm.GetAllComponents(currentEntity)) {
        if (ImGui::TreeNode(ref.description().name.c_str())) {
            ImGui::TreePop();
            ImGui::Indent();
            Value value = ref;
            draw(value, "oui");
            ref = value;
            ImGui::Unindent();
        }
    }
}

void ComponentEditor::addComponent(const Description &description)
{
    auto &cm = m_sceneManager.getCurrentLevel().getComponentManager();
    auto id = cm.GetComponentId(description.name).value();
    Value newComponent = createValue(description.type);
    cm.AddComponent(currentEntity, newComponent, id);
}
