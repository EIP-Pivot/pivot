#pragma once

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>

#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/engine.hxx>
#include <pivot/graphics/VulkanApplication.hxx>
#include <pivot/graphics/types/common.hxx>

class Editor
{
public:
    Editor(const pivot::ecs::SceneManager &sceneManager, pivot::ecs::CurrentScene scene)
        : m_sceneManager(sceneManager),
          m_currentScene(scene),
          run(false),
          currentGizmoOperation(ImGuizmo::TRANSLATE),
          currentGizmoMode(ImGuizmo::LOCAL),
          useSnap(false),
          aspectRatio(0.f){};
    void create(pivot::Engine &engine, pivot::graphics::PipelineStorage &pipelineStorage);
    pivot::ecs::SceneManager::SceneId addScene(pivot::Engine &engine);
    pivot::ecs::SceneManager::SceneId addScene(pivot::Engine &engine, std::string name);

    bool getRun();
    void setAspectRatio(float aspect);
    void DisplayGuizmo(Entity entity, const pivot::builtins::Camera &camera, const ImVec2 &offset, const ImVec2 &size);

private:
    void createPopUp(pivot::Engine &engine);

    const pivot::ecs::SceneManager &m_sceneManager;
    pivot::ecs::CurrentScene m_currentScene;
    bool shouldForce = false;

    bool run;
    ImGuizmo::OPERATION currentGizmoOperation;
    ImGuizmo::MODE currentGizmoMode;
    bool useSnap;
    float aspectRatio;
    float snap[3] = {20.f, 20.f, 20.f};
    std::vector<std::string> availableModes;
};
