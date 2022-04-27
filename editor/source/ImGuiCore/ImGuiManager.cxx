#include "ImGuiCore/ImGuiManager.hxx"

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <Logger.hpp>
#include <nfd.hpp>

void ImGuiManager::newFrame(pivot::Engine &engine)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    ImGui::Begin("Load/Save");
    saveScene(engine);
    loadScene(engine);
    ImGui::End();
}

void ImGuiManager::saveScene(pivot::Engine &engine)
{
    if (ImGui::Button("Save Scene")) {
        NFD::Guard nfd_guard;
        NFD::UniquePath savePath;
        nfdfilteritem_t filterItemSave[] = {{"Scene", "json"}};
        auto filename = m_sceneManager.getCurrentScene().getName() + ".json";
        auto resultSave = NFD::SaveDialog(savePath, filterItemSave, 1, nullptr, filename.c_str());

        switch (resultSave) {
            case NFD_OKAY: {
                logger.info("Save Scene") << savePath;
                engine.saveScene(m_sceneManager.getCurrentSceneId(), savePath.get());
                ImGui::OpenPopup("Save");
            } break;
            case NFD_ERROR: {
                logger.err("File Dialog") << NFD::GetError();
                NFD::ClearError();
            } break;
            case NFD_CANCEL: break;
        }
    }

    if (ImGui::BeginPopupModal("Save", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Scene correctly saved");
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

void ImGuiManager::loadScene(pivot::Engine &engine)
{
    static std::string loading_result;
    if (ImGui::Button("Load Scene")) {
        NFD::Guard nfd_guard;
        NFD::UniquePath scenePath;
        nfdfilteritem_t filterItemSave[] = {{"Scene", "json"}};
        auto filename = m_sceneManager.getCurrentScene().getName() + ".json";
        auto resultSave = NFD::OpenDialog(scenePath, filterItemSave, 1);

        loading_result.clear();
        switch (resultSave) {
            case NFD_OKAY: {
                logger.info("Load Scene") << scenePath;
                ImGui::OpenPopup("Load");
                try {
                    engine.loadScene(scenePath.get());
                    loading_result = "Scene correctly loaded";
                } catch (const std::exception &e) {
                    logger.err() << e.what();
                    loading_result = e.what();
                    ImGui::OpenPopup("Load");
                }
            } break;
            case NFD_ERROR: {
                logger.err("File Dialog") << NFD::GetError();
                NFD::ClearError();
            } break;
            case NFD_CANCEL: break;
        }
    }
    if (ImGui::BeginPopupModal("Load", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%s", loading_result.c_str());
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

void ImGuiManager::render() { ImGui::Render(); }
