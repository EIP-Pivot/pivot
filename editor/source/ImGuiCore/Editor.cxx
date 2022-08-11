#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "ImGuiCore/Editor.hxx"
#include "ImGuiCore/ImGuiTheme.hxx"

using namespace pivot::ecs;

void Editor::create(pivot::Engine &engine, pivot::graphics::PipelineStorage &pipelineStorage)
{
    ImVec2 sceneSize = ImVec2(50, 15);
    ImVec2 newSceneSize = ImVec2(20, 15);
    ImGuiIO &io = ImGui::GetIO();
    ImGui::Begin(" Editor ");
    ImGuiTheme::setDefaultFramePadding();
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
        //        ImGui::Separator();
        //        ImGui::Checkbox("##", &useSnap);
        //        ImGui::SameLine();
        //        switch (currentGizmoOperation) {
        //            case ImGuizmo::TRANSLATE: ImGui::InputFloat3("Snap", &snap[0]); break;
        //            case ImGuizmo::ROTATE: ImGui::InputFloat("Angle Snap", &snap[0]); break;
        //            case ImGuizmo::SCALE: ImGui::InputFloat("Scale Snap", &snap[0]); break;
        //            default: break;
        //        }
        //        ImGui::Separator();
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
    ImGuiTheme::unsetDefaultFramePadding();
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
