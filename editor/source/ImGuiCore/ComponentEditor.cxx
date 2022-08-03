#include "ImGuiCore/ComponentEditor.hxx"
#include "ImGuiCore/TypeTemplate/Template.hxx"
#include <magic_enum.hpp>

using namespace pivot::ecs;
using namespace pivot::ecs::component;
using namespace pivot::ecs::data;

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

void ComponentEditor::createPopUp()
{
    auto &cm = m_scene->getComponentManager();
    if (ImGui::BeginPopup("AddComponent")) {
        for (const auto &[name, description]: m_index) {
            auto id = cm.GetComponentId(name);
            if (!id || cm.GetComponent(currentEntity, *id) == std::nullopt) {
                if (ImGui::MenuItem(name.c_str())) {
                    if (!id) { cm.RegisterComponent(description); }
                    addComponent(description);
                }
            }
        }
        ImGui::EndPopup();
    }
}

void ComponentEditor::displayComponent()
{
    auto &cm = m_scene->getComponentManager();
    for (ComponentRef ref: cm.GetAllComponents(currentEntity)) {
        if (ImGui::TreeNode(ref.description().name.c_str())) {
            ImGui::TreePop();
            ImGui::Indent();
            Value value = ref;
            draw(value, "oui");
            ref = value;
            if (ref.description().name != "Tag") {
                if (ImGui::Button("Remove")) {
                    auto id = cm.GetComponentId(ref.description().name).value();
                    cm.RemoveComponent(currentEntity, id);
                }
            }
            ImGui::Unindent();
        }
    }
}

void ComponentEditor::addComponent(const Description &description)
{
    auto &cm = m_scene->getComponentManager();
    auto id = cm.GetComponentId(description.name).value();
    Value newComponent = description.defaultValue;
    cm.AddComponent(currentEntity, newComponent, id);
}
