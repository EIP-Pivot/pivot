#include <imgui.h>
// Must be after imgui
#include <ImGuizmo.h>

#include <glm/gtc/type_ptr.hpp>
#include <misc/cpp/imgui_stdlib.h>

#include "ImGuiCore/Editor.hxx"

using namespace pivot::ecs;

void Editor::create(pivot::Engine &engine, pivot::graphics::PipelineStorage &pipelineStorage)
{
    ImVec2 sceneSize = ImVec2(50, 15);
    ImVec2 newSceneSize = ImVec2(20, 15);
    ImGuiIO &io = ImGui::GetIO();
    ImGui::Begin("Editor");
    if (!run) {
        createPopUp(engine);
        for (SceneManager::SceneId sceneId = 0; sceneId < m_sceneManager.getLivingScene(); sceneId++) {
            if (ImGui::Selectable(m_sceneManager.getSceneById(sceneId).getName().c_str(),
                                  m_sceneManager.getCurrentSceneId() == sceneId, 0, sceneSize)) {
                engine.changeCurrentScene(sceneId);
            }
            ImGui::SameLine();
        }
        if (ImGui::Selectable(" +", false, 0, newSceneSize)) { ImGui::OpenPopup("AddScene"); }
        ImGui::Separator();
        if (ImGui::Button("Save")) {
            // TODO: Use file dialog to select path
            engine.saveScene(m_sceneManager.getCurrentSceneId(), "scene.json");
        }
        ImGui::Separator();
        if (ImGui::RadioButton("Translate", currentGizmoOperation == ImGuizmo::TRANSLATE))
            currentGizmoOperation = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", currentGizmoOperation == ImGuizmo::ROTATE))
            currentGizmoOperation = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", currentGizmoOperation == ImGuizmo::SCALE))
            currentGizmoOperation = ImGuizmo::SCALE;
        ImGui::Separator();
        if (ImGuizmo::IsUsing()) {
            ImGui::Text("Using gizmo");
        } else {
            ImGui::Text(ImGuizmo::IsOver() ? "Over gizmo" : "");
            ImGui::SameLine();
            ImGui::Text(ImGuizmo::IsOver(ImGuizmo::TRANSLATE) ? "Over translate gizmo" : "");
            ImGui::SameLine();
            ImGui::Text(ImGuizmo::IsOver(ImGuizmo::ROTATE) ? "Over rotate gizmo" : "");
            ImGui::SameLine();
            ImGui::Text(ImGuizmo::IsOver(ImGuizmo::SCALE) ? "Over scale gizmo" : "");
        }
        ImGui::Separator();
        if (currentGizmoOperation != ImGuizmo::SCALE) {
            if (ImGui::RadioButton("Local", currentGizmoMode == ImGuizmo::LOCAL)) currentGizmoMode = ImGuizmo::LOCAL;
            ImGui::SameLine();
            if (ImGui::RadioButton("World", currentGizmoMode == ImGuizmo::WORLD)) currentGizmoMode = ImGuizmo::WORLD;
        } else {
            currentGizmoMode = ImGuizmo::LOCAL;
        }
        ImGui::Checkbox("##", &useSnap);
        ImGui::SameLine();
        switch (currentGizmoOperation) {
            case ImGuizmo::TRANSLATE: ImGui::InputFloat3("Snap", &snap[0]); break;
            case ImGuizmo::ROTATE: ImGui::InputFloat("Angle Snap", &snap[0]); break;
            case ImGuizmo::SCALE: ImGui::InputFloat("Scale Snap", &snap[0]); break;
            default: break;
        }
        ImGui::Separator();
    }
    ImGui::Checkbox("Should force pipeline ?", &shouldForce);
    if (ImGui::BeginCombo("##sample_count", pipelineStorage.getDefaultName().c_str())) {
        for (const auto &msaa: pipelineStorage.getNames()) {
            bool is_selected = (pipelineStorage.getDefaultName() == msaa);
            if (ImGui::Selectable(msaa.c_str(), is_selected)) { pipelineStorage.setDefault(msaa, shouldForce); }
            if (is_selected) { ImGui::SetItemDefaultFocus(); }
        }
        ImGui::EndCombo();
    }

    ImGui::Text("X: %f Y: %f", io.MousePos.x, io.MousePos.y);
    ImGui::Text("Fps: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("ms/frame %.3f", 1000.0f / ImGui::GetIO().Framerate);
#ifdef CULLING_DEBUG
    ImGui::Text("Culling separated from camera: %s", std::to_string(!cullingCameraFollowsCamera).c_str());
#endif
    ImGui::Checkbox("Systems", &run);
    ImGui::End();
}

SceneManager::SceneId Editor::addScene(pivot::Engine &engine)
{
    SceneManager::SceneId newScene = engine.registerScene();
    engine.changeCurrentScene(newScene);
    // m_sceneManager.getCurrentLevel().Init();
    return newScene;
}

SceneManager::SceneId Editor::addScene(pivot::Engine &engine, std::string name)
{
    SceneManager::SceneId newScene = engine.registerScene(name);
    engine.changeCurrentScene(newScene);
    // m_sceneManager.getCurrentLevel().Init();
    return newScene;
}

bool Editor::getRun() { return run; }

void Editor::setAspectRatio(float aspect) { aspectRatio = aspect; }

void Editor::DisplayGuizmo(Entity entity)
{
    // const auto view = camera.getView();
    // const auto projection = camera.getProjection(80.9f, aspectRatio);

    // const float *view_ptr = glm::value_ptr(view);
    // const float *projection_ptr = glm::value_ptr(projection);
    // float *matrix = glm::value_ptr(m_sceneManager.getCurrentLevel()
    //                                    .GetComponent<RenderObject>(entity)
    //                                    .objectInformation.transform.getModelMatrix());
    // ImGuiIO &io = ImGui::GetIO();
    // ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    // ImGuizmo::Manipulate(view_ptr, projection_ptr, currentGizmoOperation, currentGizmoMode, matrix, NULL,
    //                      useSnap ? &snap[0] : NULL);
}

void Editor::createPopUp(pivot::Engine &engine)
{
    if (ImGui::BeginPopup("AddScene")) {
        static std::string sceneName;
        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText("##", &sceneName, ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (sceneName.empty())
                addScene(engine);
            else
                addScene(engine, sceneName);
            sceneName.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
