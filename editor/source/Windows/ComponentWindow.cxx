#include "Windows/ComponentWindow.hxx"

#include <imgui.h>

#include "ImGuiCore/ImGuiTheme.hxx"
#include "ImGuiCore/TypeTemplate/Template.hxx"
#include <magic_enum.hpp>

using namespace pivot::editor;
using namespace pivot::ecs;
using namespace pivot::ecs::component;
using namespace pivot::ecs::data;

void ComponentWindow::render()
{
    PROFILE_FUNCTION();
    ImGui::Begin(" Component editor ", &m_open);
    if (m_manager.getSelectedEntity() != -1) {
        ImGuiTheme::setDefaultFramePadding();
        createPopUp();
        displayComponent();
        if (CustomWidget::ButtonCenteredOnLine("Add Component")) { ImGui::OpenPopup("AddComponent"); }
        ImGuiTheme::unsetDefaultFramePadding();
    } else {
        ImGui::Text("No entity selected.");
    }
    ImGui::End();
}

void ComponentWindow::createPopUp()
{
    PROFILE_FUNCTION();
    auto &cm = m_manager.getCurrentScene()->getComponentManager();
    if (ImGui::BeginPopup("AddComponent")) {
        for (const auto &[name, description]: m_manager.getComponentIndex()) {
            auto id = cm.GetComponentId(name);
            if (!id || cm.GetComponent(m_manager.getSelectedEntity(), *id) == std::nullopt) {
                if (ImGui::MenuItem(name.c_str())) {
                    if (!id) { cm.RegisterComponent(description); }
                    addComponent(description);
                }
            }
        }
        ImGui::EndPopup();
    }
}

void ComponentWindow::displayComponent()
{
    PROFILE_FUNCTION();
    auto &cm = m_manager.getCurrentScene()->getComponentManager();
    displayName();
    for (ComponentRef ref: cm.GetAllComponents(m_manager.getSelectedEntity())) {
        if (ref.description().name == "Tag") continue;
        deleteComponent(ref);
        if (ImGui::TreeNode(ref.description().name.c_str())) {
            ImGui::TreePop();
            Value value = ref;
            ImGui::PushID(ref.description().name.c_str());
            draw(value, "oui");
            ImGui::PopID();
            ref.set(value);
            ImGui::Separator();
        }
    }
}

void ComponentWindow::displayName()
{
    PROFILE_FUNCTION();
    auto &cm = m_manager.getCurrentScene()->getComponentManager();
    auto tagId = cm.GetComponentId("Tag").value();
    auto &tagArray = cm.GetComponentArray(tagId);
    auto tag = ComponentRef(tagArray, m_manager.getSelectedEntity());
    Value value = tag;
    auto &name = std::get<std::string>(std::get<Record>(value).at("name"));
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##Tag", &name);
    ImGui::PopItemWidth();
    tag.set(value);
}

void ComponentWindow::deleteComponent(ComponentRef ref)
{
    PROFILE_FUNCTION();
    auto &cm = m_manager.getCurrentScene()->getComponentManager();
    ImGuiIO &io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];
    float lineHeight = (GImGui->Font->FontSize * boldFont->Scale) + GImGui->Style.FramePadding.y * 2.f;
    ImVec2 buttonSize = {lineHeight + 3.f, lineHeight};
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.f});
    ImGui::PushFont(boldFont);
    ImGui::PushID((std::string("Delete") + ref.description().name).c_str());
    if (ImGui::Button("X", buttonSize)) {
        auto id = cm.GetComponentId(ref.description().name).value();
        cm.RemoveComponent(m_manager.getSelectedEntity(), id);
    }
    ImGui::PopID();
    ImGui::PopFont();
    ImGui::PopStyleColor(3);
    ImGui::SameLine();
}

void ComponentWindow::addComponent(const Description &description)
{
    PROFILE_FUNCTION();
    auto &cm = m_manager.getCurrentScene()->getComponentManager();
    auto id = cm.GetComponentId(description.name).value();
    Value newComponent = description.defaultValue;
    cm.AddComponent(m_manager.getSelectedEntity(), newComponent, id);
}
