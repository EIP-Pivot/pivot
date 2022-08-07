#include "ImGuiCore/ComponentEditor.hxx"
#include "ImGuiCore/TypeTemplate/Template.hxx"
#include <magic_enum.hpp>

#include <imgui.h>

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
        if (ref.description().name != "Tag") deleteComponent(ref);
        if (ImGui::TreeNode(ref.description().name.c_str())) {
            ImGui::TreePop();
            Value value = ref;
            draw(value, "oui");
            ref = value;
            ImGui::Separator();
        }
    }
}

void ComponentEditor::deleteComponent(ComponentRef ref)
{
    auto &cm = m_scene->getComponentManager();
    ImGuiIO &io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];
    float lineHeight = (GImGui->Font->FontSize * boldFont->Scale) + GImGui->Style.FramePadding.y * 2.f;
    ImVec2 buttonSize = {lineHeight + 3.f, lineHeight};
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.f});
    ImGui::PushFont(boldFont);
    if (ImGui::Button("X", buttonSize)) {
        auto id = cm.GetComponentId(ref.description().name).value();
        cm.RemoveComponent(currentEntity, id);
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);
    ImGui::SameLine();
}

void ComponentEditor::addComponent(const Description &description)
{
    auto &cm = m_scene->getComponentManager();
    auto id = cm.GetComponentId(description.name).value();
    Value newComponent = description.defaultValue;
    cm.AddComponent(currentEntity, newComponent, id);
}
