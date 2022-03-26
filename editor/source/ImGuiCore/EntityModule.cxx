#include "ImGuiCore/EntityModule.hxx"
#include <misc/cpp/imgui_stdlib.h>
#include <pivot/graphics/DebugMacros.hxx>

extern SceneManager gSceneManager;

void EntityModule::create()
{
    auto &componentManager = gSceneManager.getCurrentLevel().getComponentManager();
    auto tagId = componentManager.GetComponentId("Tag").value();
    if (gSceneManager.getCurrentLevelId() != currentScene) {
        _hasSelected = false;
        entitySelected = -1;
    }
    currentScene = gSceneManager.getCurrentLevelId();
    ImGui::Begin("Entity");
    createPopUp();
    for (auto const &[entity, _]: gSceneManager.getCurrentLevel().getEntities()) {
        if (ImGui::Selectable(
                std::get<std::string>(
                    std::get<pivot::ecs::data::Record>(componentManager.GetComponent(entity, tagId).value()).at("name"))
                    .c_str(),
                entitySelected == entity)) {
            _hasSelected = true;
            entitySelected = entity;
        }
    }
    ImGui::Separator();
    if (ImGui::Button("Add entity")) ImGui::OpenPopup("NewEntity");
    if (_hasSelected) {
        ImGui::SameLine();
        if (ImGui::Button("Remove entity")) removeEntity();
    }
    ImGui::End();
}

Entity EntityModule::addEntity()
{
    Entity newEntity = gSceneManager.getCurrentLevel().CreateEntity();
    entitySelected = newEntity;
    _hasSelected = true;
    return newEntity;
}

Entity EntityModule::addEntity(std::string name)
{
    Entity newEntity = gSceneManager.getCurrentLevel().CreateEntity(name);
    entitySelected = newEntity;
    _hasSelected = true;
    return newEntity;
}

void EntityModule::removeEntity()
{
    gSceneManager.getCurrentLevel().DestroyEntity(entitySelected);
    entitySelected = 0;
    _hasSelected = false;
}

Entity EntityModule::getEntitySelected() { return entitySelected; }

bool EntityModule::hasSelected() { return _hasSelected; }

void EntityModule::createPopUp()
{
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
