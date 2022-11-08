#pragma once

#include <imgui.h>

#include <ImGuizmo.h>

#include "ImGuiCore/AssetBrowser.hxx"
#include "ImGuiCore/ImGuiManager.hxx"
#include <pivot/internal/LocationCamera.hxx>

class SceneEditor
{
public:
    SceneEditor(ImGuiManager &imGuiManager, pivot::ecs::CurrentScene scene)
        : m_imGuiManager(imGuiManager),
          m_currentScene(scene),
          currentGizmoOperation(ImGuizmo::TRANSLATE),
          currentGizmoMode(ImGuizmo::LOCAL),
          aspectRatio(0.f),
          useSnap(false){};
    void create();
    void setAspectRatio(float aspect);
    void DisplayGuizmo(Entity entity, const pivot::internals::LocationCamera &camera);

public:
    ImVec2 offset;
    ImVec2 size;

private:
    void toolbar();
    void imGuizmoOperation();
    void imGuizmoMode();
    void viewport();

private:
    ImGuiManager &m_imGuiManager;
    pivot::ecs::CurrentScene m_currentScene;
    ImGuizmo::OPERATION currentGizmoOperation;
    ImGuizmo::MODE currentGizmoMode;
    float aspectRatio;
    bool useSnap;
    float snap[3] = {20.f, 20.f, 20.f};
};
