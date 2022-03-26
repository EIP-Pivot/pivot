#include "ImGuiCore/Editor.hxx"
#include <glm/gtc/type_ptr.hpp>
#include <misc/cpp/imgui_stdlib.h>

extern SceneManager gSceneManager;

void Editor::create()
{
    ImVec2 sceneSize = ImVec2(50, 15);
    ImVec2 newSceneSize = ImVec2(20, 15);
    ImGuiIO &io = ImGui::GetIO();
    ImGui::Begin("Editor");
    if (!run) {
        createPopUp();
        for (LevelId sceneId = 0; sceneId < gSceneManager.getLivingScene(); sceneId++) {
            if (ImGui::Selectable(gSceneManager.getLevelById(sceneId).getName().c_str(),
                                  gSceneManager.getCurrentLevelId() == sceneId, 0, sceneSize)) {
                gSceneManager.setCurrentLevelId(sceneId);
            }
            ImGui::SameLine();
        }
        if (ImGui::Selectable(" +", false, 0, newSceneSize)) { ImGui::OpenPopup("AddScene"); }
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
        }
        ImGui::Separator();
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

LevelId Editor::addScene()
{
    LevelId newScene = gSceneManager.registerLevel();
    gSceneManager.setCurrentLevelId(newScene);
    // gSceneManager.getCurrentLevel().Init();
    return newScene;
}

LevelId Editor::addScene(std::string name)
{
    LevelId newScene = gSceneManager.registerLevel(name);
    gSceneManager.setCurrentLevelId(newScene);
    // gSceneManager.getCurrentLevel().Init();
    return newScene;
}

bool Editor::getRun() { return run; }

Camera &Editor::getCamera() { return camera; }

void Editor::setAspectRatio(float aspect) { aspectRatio = aspect; }

void Editor::DisplayGuizmo(Entity entity)
{
    // const auto view = camera.getView();
    // const auto projection = camera.getProjection(80.9f, aspectRatio);

    // const float *view_ptr = glm::value_ptr(view);
    // const float *projection_ptr = glm::value_ptr(projection);
    // float *matrix = glm::value_ptr(gSceneManager.getCurrentLevel()
    //                                    .GetComponent<RenderObject>(entity)
    //                                    .objectInformation.transform.getModelMatrix());
    // ImGuiIO &io = ImGui::GetIO();
    // ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    // ImGuizmo::Manipulate(view_ptr, projection_ptr, currentGizmoOperation, currentGizmoMode, matrix, NULL,
    //                      useSnap ? &snap[0] : NULL);
}

void Editor::createPopUp()
{
    if (ImGui::BeginPopup("AddScene")) {
        static std::string sceneName;
        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText("##", &sceneName, ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (sceneName.empty())
                addScene();
            else
                addScene(sceneName);
            sceneName.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
