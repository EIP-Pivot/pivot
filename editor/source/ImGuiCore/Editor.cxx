#include <imgui.h>
// Must be after imgui
#include <ImGuizmo.h>

#include <glm/gtc/type_ptr.hpp>
#include <misc/cpp/imgui_stdlib.h>

#include <numbers>

#include <pivot/builtins/components/Transform.hxx>

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

void Editor::DisplayGuizmo(Entity entity, const pivot::builtins::Camera &camera)
{
    using Transform = pivot::builtins::components::Transform;

    const auto view = camera.getView();
    const auto projection = camera.getProjection(pivot::Engine::fov, aspectRatio);

    const float *view_ptr = glm::value_ptr(view);
    const float *projection_ptr = glm::value_ptr(projection);

    // TODO: Refactor this out, to compute only when the scene changes
    auto &cm = m_currentScene->getComponentManager();
    auto &array = cm.GetComponentArray(cm.GetComponentId(Transform::description.name).value());
    auto &ro_array = dynamic_cast<pivot::ecs::component::DenseTypedComponentArray<pivot::graphics::Transform> &>(array);
    if (!ro_array.entityHasValue(entity)) return;
    pivot::graphics::Transform &transform = ro_array.getData()[entity];
    auto matrix = transform.getModelMatrix();
    float *matrix_data = glm::value_ptr(matrix);
    ImGuiIO &io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    glm::mat4x4 delta_matrix;
    bool changed = ImGuizmo::Manipulate(view_ptr, projection_ptr, currentGizmoOperation, currentGizmoMode, matrix_data,
                                        glm::value_ptr(delta_matrix), useSnap ? &snap[0] : NULL);
    if (!changed) return;

    glm::vec3 rotation_deg;
    ImGuizmo::DecomposeMatrixToComponents(matrix_data, glm::value_ptr(transform.position), glm::value_ptr(rotation_deg),
                                          glm::value_ptr(transform.scale));
    transform.rotation = rotation_deg * std::numbers::pi_v<float> / 180.0f;
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
