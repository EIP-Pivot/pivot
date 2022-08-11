#include <utility>

#include "ImGuiCore/AssetBrowser.hxx"
#include "ImGuiCore/CustomWidget.hxx"

using namespace pivot::ecs::data;

void AssetBrowser::create()
{
    ImGui::Begin(" Asset ");
    auto image = m_imGuiManager.getTextureId("Prefab");
    std::size_t imagesMax = ImGui::GetWindowWidth() / (70 + ImGui::GetStyle().ItemSpacing.x);
    std::size_t images = 0;
    for (const auto &name: m_assetStorage.getPrefabs()) {
        CustomWidget::ImageText(image, {70, 70}, name);
        dropSource(name);
        images++;
        if (imagesMax != 0 && images % imagesMax != 0) ImGui::SameLine();
    }
    ImGui::End();
}

void AssetBrowser::dropSource(const std::string &name)
{
    auto image = m_imGuiManager.getTextureId("Prefab");
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        struct wrapper my_wrapper = {*this};
        std::memcpy(my_wrapper.name, name.c_str(), name.size());
        ImGui::SetDragDropPayload("ASSET", &my_wrapper, sizeof(my_wrapper));
        CustomWidget::ImageText(image, {70, 70}, name);
        ImGui::EndDragDropSource();
    }
}

void AssetBrowser::createEntity(std::string_view name)
{
    Entity newEntity = m_scene->CreateEntity();
    auto &cm = m_scene->getComponentManager();
    auto transformIndex = cm.GetComponentId("Transform").value();
    Value transform = Value{
        Record{{"position", Value{glm::vec3{}}}, {"rotation", Value{glm::vec3{}}}, {"scale", Value{glm::vec3{1.f}}}}};
    cm.AddComponent(newEntity, transform, transformIndex);
    auto renderObjectIndex = cm.GetComponentId("RenderObject").value();
    Value renderObject =
        Value{Record{{"meshID", Asset{std::string(name)}}, {"pipelineID", ""}, {"materialIndex", Asset{""}}}};
    cm.AddComponent(newEntity, renderObject, renderObjectIndex);
}
