#include "Windows/EntityWindow.hxx"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "ImGuiCore/ImGuiTheme.hxx"

using namespace pivot::editor;

void EntityWindow::render()
{
    PROFILE_FUNCTION();
    auto &componentManager = m_manager.getCurrentScene()->getComponentManager();
    auto tagId = componentManager.GetComponentId("Tag").value();
    ImGui::Begin(" Entity ", &m_open);
    if (ImGui::IsKeyPressed(ImGuiKey_LeftCtrl)) {
        if (ImGui::IsKeyPressed(ImGuiKey_D)) {
            addEntity(m_manager.getCurrentScene()->getEntityName(m_manager.getSelectedEntity()) + " - Copied");
        }
    }
    ImGuiTheme::setDefaultFramePadding();
    createPopUp();
    if (ImGui::Button("Add entity")) ImGui::OpenPopup("NewEntity");
    if (m_manager.getSelectedEntity() != -1u) {
        ImGui::SameLine();
        if (ImGui::Button("Remove entity")) removeEntity();
    }
    ImGui::Separator();
    for (auto const &[entity, _]: m_manager.getCurrentScene()->getEntities()) {
        if (ImGui::Selectable(
                std::get<std::string>(
                    std::get<pivot::ecs::data::Record>(componentManager.GetComponent(entity, tagId).value()).at("name"))
                    .c_str(),
                m_manager.getSelectedEntity() == entity)) {
            m_manager.setSelectedEntity(entity);
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            std::cout << std::get<std::string>(
                             std::get<pivot::ecs::data::Record>(componentManager.GetComponent(entity, tagId).value())
                                 .at("name"))
                             .c_str()
                      << std::endl;
        }
    }
    ImGuiTheme::unsetDefaultFramePadding();
    ImGui::End();
}

Entity EntityWindow::addEntity()
{
    PROFILE_FUNCTION();
    Entity newEntity = m_manager.getCurrentScene()->CreateEntity();
    m_manager.setSelectedEntity(newEntity);
    return newEntity;
}

Entity EntityWindow::addEntity(std::string name)
{
    PROFILE_FUNCTION();
    Entity newEntity = m_manager.getCurrentScene()->CreateEntity(name);
    m_manager.setSelectedEntity(newEntity);
    return newEntity;
}

void EntityWindow::removeEntity()
{
    PROFILE_FUNCTION();
    m_manager.getCurrentScene()->DestroyEntity(m_manager.getSelectedEntity());
    m_manager.setSelectedEntity(-1);
}

void EntityWindow::createPopUp()
{
    PROFILE_FUNCTION();
    if (ImGui::BeginPopup("NewEntity")) {
        static std::string entityName;
        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText("##", &entityName, ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (entityName.empty())
                addEntity();
            else
                addEntity(entityName);
            entityName.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
