#include <utility>

#include "ImGuiCore/AssetBrowser.hxx"

using namespace pivot::ecs::data;

void AssetBrowser::create()
{
    ImGui::Begin("Asset");
    for (const auto &name: m_assetStorage.getPrefabs()) {
        ImGui::Button(name.c_str());
        dropSource(name);
    }
    ImGui::End();
}

void AssetBrowser::dropSource(const std::string &name)
{
    if (ImGui::BeginDragDropSource()) {
        struct wrapper my_wrapper = {*this, name};
        ImGui::SetDragDropPayload("ASSET", &my_wrapper, sizeof(my_wrapper));
        ImGui::Button(name.c_str());
        ImGui::EndDragDropSource();
    }
}

void AssetBrowser::createEntity(std::string name)
{
    Entity newEntity = m_scene->CreateEntity();
    auto &cm = m_scene->getComponentManager();
    auto transformIndex = cm.GetComponentId("Transform").value();
    Value transform = Value{
        Record{{"position", Value{glm::vec3{}}}, {"rotation", Value{glm::vec3{}}}, {"scale", Value{glm::vec3{1.f}}}}};
    cm.AddComponent(newEntity, transform, transformIndex);
    auto renderObjectIndex = cm.GetComponentId("RenderObject").value();
    Value renderObject =
        Value{Record{{"meshID", Asset{std::move(name)}}, {"pipelineID", ""}, {"materialIndex", Asset{""}}}};
    cm.AddComponent(newEntity, renderObject, renderObjectIndex);
}
