#include "Windows/AssetWindow.hxx"

#include <imgui.h>

#include "ImGuiCore/CustomWidget.hxx"
#include "ImGuiCore/ImGuiTheme.hxx"

using namespace pivot::editor;
using namespace pivot::ecs::data;

void AssetWindow::render()
{
    PROFILE_FUNCTION();
    ImGui::Begin(" Asset ", &m_open);
    ImGuiTheme::setDefaultFramePadding();
    auto image = m_manager.getTextureId("Prefab");
    std::size_t imagesMax = ImGui::GetWindowWidth() / (70 + ImGui::GetStyle().ItemSpacing.x);
    std::size_t images = 0;
    for (const auto &name: m_manager.getAssetStorage().getPrefabs()) {
        CustomWidget::ImageText(image, {70, 70}, name);
        dropSource(name);
        images++;
        if (imagesMax != 0 && images % imagesMax != 0) ImGui::SameLine();
    }
    ImGuiTheme::unsetDefaultFramePadding();
    ImGui::End();
}

void AssetWindow::createEntity(std::string_view name)
{
    PROFILE_FUNCTION();
    Entity newEntity = m_manager.getCurrentScene()->CreateEntity();
    auto &cm = m_manager.getCurrentScene()->getComponentManager();
    auto transformIndex = cm.GetComponentId("Transform").value();
    Value transform = Value{
        Record{{"position", Value{glm::vec3{}}}, {"rotation", Value{glm::vec3{}}}, {"scale", Value{glm::vec3{1.f}}}}};
    cm.AddComponent(newEntity, transform, transformIndex);
    auto renderObjectIndex = cm.GetComponentId("RenderObject").value();
    Value renderObject =
        Value{Record{{"meshID", Asset{std::string(name)}}, {"pipelineID", ""}, {"materialIndex", Asset{""}}}};
    cm.AddComponent(newEntity, renderObject, renderObjectIndex);
}

void AssetWindow::dropSource(const std::string &name)
{
    PROFILE_FUNCTION();
    auto image = m_manager.getTextureId("Prefab");
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        struct wrapper my_wrapper = {*this};
        std::memcpy(my_wrapper.name, name.c_str(), name.size());
        ImGui::SetDragDropPayload("ASSET", &my_wrapper, sizeof(my_wrapper));
        CustomWidget::ImageText(image, {70, 70}, name);
        ImGui::EndDragDropSource();
    }
}
