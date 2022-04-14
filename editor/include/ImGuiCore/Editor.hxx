#pragma once

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>

#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/graphics/VulkanApplication.hxx>
#include <pivot/graphics/types/common.hxx>

class Editor
{
public:
    Editor(pivot::ecs::SceneManager &sceneManager)
        : m_sceneManager(sceneManager),
          run(false),
          camera(Camera(glm::vec3(0, 200, 500))),
          currentGizmoOperation(ImGuizmo::TRANSLATE),
          currentGizmoMode(ImGuizmo::LOCAL),
          useSnap(false),
          aspectRatio(0.f){};

    void init(pivot::graphics::VulkanApplication &app)
    {
        availableModes = app.pipelineStorage.getNames();
        storage = app.pipelineStorage;
    }
    void create();
    LevelId addScene();
    LevelId addScene(std::string name);

    bool getRun();
    Camera &getCamera();
    void setAspectRatio(float aspect);
    void DisplayGuizmo(Entity entity);

private:
    void createPopUp();

    pivot::ecs::SceneManager &m_sceneManager;
    OptionalRef<pivot::graphics::PipelineStorage> storage;
    bool shouldForce = false;

    bool run;
    Camera camera;
    ImGuizmo::OPERATION currentGizmoOperation;
    ImGuizmo::MODE currentGizmoMode;
    bool useSnap;
    float aspectRatio;
    float snap[3] = {20.f, 20.f, 20.f};
    std::vector<std::string> availableModes;

#ifdef CULLING_DEBUG
public:
    Camera cullingCamera;
    bool cullingCameraFollowsCamera = true;

private:
#endif
};
