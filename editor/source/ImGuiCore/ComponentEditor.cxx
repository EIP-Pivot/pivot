#include "ImGuiCore/ComponentEditor.hxx"
#include "ImGuiCore/AssetBrowser.hxx"
#include "ImGuiCore/TypeTemplate/Template.hxx"
#include <magic_enum.hpp>
#include <misc/cpp/imgui_stdlib.h>

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
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, m_dropColor);

        ImGui::BeginChild("DropZone", ImVec2(0, 0), true);

        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) &&
            ImGui::GetDragDropPayload() != nullptr) {
            m_dropColor = ImVec4(ImColor(110, 110, 110));
        } else {
            m_dropColor = ImVec4(ImColor(66, 66, 66));
        }

        std::string text = "DropZone";
        auto windowWidth = ImGui::GetWindowSize().x;
        auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

        auto windowHeight = ImGui::GetWindowSize().y;
        auto textHeight = ImGui::CalcTextSize(text.c_str()).y;

        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::SetCursorPosY((windowHeight - textHeight) * 0.5f);
        ImGui::Text("%s", text.c_str());

        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }
    ImGui::PushStyleColor(ImGuiCol_DragDropTarget, ImVec4(ImColor(0, 0, 0, 0)));
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET")) {
            pivot_assert(payload->Data, "Empty drag and drop payload");
            auto *assetWrapper = reinterpret_cast<AssetBrowser::wrapper *>(payload->Data);
            assetWrapper->assetBrowser.createEntity(assetWrapper->asset_name);
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::PopStyleColor();

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
