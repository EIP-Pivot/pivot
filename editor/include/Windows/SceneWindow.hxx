#pragma once

#include <imgui.h>

#include <ImGuizmo.h>

#include "Windows/IWindow.hxx"
#include "WindowsManager.hxx"

namespace pivot::editor
{

class SceneWindow : public IWindow
{
public:
    SceneWindow(const std::string &name, ecs::SceneManager::SceneId sceneId, WindowsManager &manager,
                pivot::graphics::PipelineStorage &pipelineStorage, const builtins::Camera &camera, bool &paused)
        : IWindow(manager, true),
          sceneStatus(SceneStatus::STOP),
          m_paused(paused),
          currentGizmoOperation(ImGuizmo::TRANSLATE),
          currentGizmoMode(ImGuizmo::LOCAL),
          aspectRatio(0.f),
          useSnap(false),
          m_camera(camera),
          m_pipelineStorage(pipelineStorage),
          m_name(name),
          m_sceneId(sceneId){};
    void render() override;
    void setAspectRatio(float aspect);

private:
    void displayGuizmo();
    void toolbar();
    void imGuizmoOperation();
    void imGuizmoMode();
    void setSceneStatus();
    void setPipelineStatus();
    void viewport();

private:
    enum SceneStatus { PLAY, PAUSE, STOP };
    SceneStatus sceneStatus;
    bool &m_paused;
    ImGuizmo::OPERATION currentGizmoOperation;
    ImGuizmo::MODE currentGizmoMode;
    float aspectRatio;
    bool useSnap;
    float snap[3] = {20.f, 20.f, 20.f};
    const builtins::Camera &m_camera;
    pivot::graphics::PipelineStorage &m_pipelineStorage;
    const std::string &m_name;
    ecs::SceneManager::SceneId m_sceneId;
};

}    // namespace pivot::editor
