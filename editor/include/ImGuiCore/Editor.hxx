#pragma once

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>

#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/graphics/types/common.hxx>

class Editor
{
public:
    Editor(pivot::ecs::SceneManager &sceneManager)
        : m_sceneManager(sceneManager),
          run(false),
          currentGizmoOperation(ImGuizmo::TRANSLATE),
          currentGizmoMode(ImGuizmo::LOCAL),
          useSnap(false),
          aspectRatio(0.f){};
    void create();
    LevelId addScene();
    LevelId addScene(std::string name);

    bool getRun();
    void setAspectRatio(float aspect);
    void DisplayGuizmo(Entity entity);

private:
    void createPopUp();

    pivot::ecs::SceneManager &m_sceneManager;

    bool run;
    ImGuizmo::OPERATION currentGizmoOperation;
    ImGuizmo::MODE currentGizmoMode;
    bool useSnap;
    float aspectRatio;
    float snap[3] = {20.f, 20.f, 20.f};

#ifdef CULLING_DEBUG
public:
    bool cullingCameraFollowsCamera = true;

private:
#endif
};
