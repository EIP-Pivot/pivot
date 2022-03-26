#pragma once

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>

#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/graphics/types/common.hxx>

class Editor
{
public:
    Editor()
        : run(false),
          camera(Camera(glm::vec3(0, 200, 500))),
          currentGizmoOperation(ImGuizmo::TRANSLATE),
          currentGizmoMode(ImGuizmo::LOCAL),
          useSnap(false),
          aspectRatio(0.f){};
    void create();
    LevelId addScene();
    LevelId addScene(std::string name);

    bool getRun();
    Camera &getCamera();
    void setAspectRatio(float aspect);
    void DisplayGuizmo(Entity entity);

private:
    void createPopUp();

    bool run;
    Camera camera;
    ImGuizmo::OPERATION currentGizmoOperation;
    ImGuizmo::MODE currentGizmoMode;
    bool useSnap;
    float aspectRatio;
    float snap[3] = {20.f, 20.f, 20.f};

#ifdef CULLING_DEBUG
public:
    Camera cullingCamera;
    bool cullingCameraFollowsCamera = true;

private:
#endif
};
