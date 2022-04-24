#include "ImGuiCore/ImGuiManager.hxx"

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <Logger.hpp>
#include <nfd.h>

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
        NFD_Init();

        nfdchar_t *savePath = nullptr;
        nfdfilteritem_t filterItemSave[] = {{"Scene", "json"}};
        auto filename = m_sceneManager.getCurrentScene().getName() + ".json";
        auto resultSave = NFD_SaveDialog(&savePath, filterItemSave, 1, NULL, filename.data());

        switch (resultSave) {
            case NFD_OKAY: {
                logger.info("Save Scene") << savePath;
                m_sceneManager.getCurrentScene().save(savePath);
                ImGui::OpenPopup("Save");
            } break;
            case NFD_ERROR: {
                logger.err("File Dialog") << NFD_GetError();
            } break;
            case NFD_CANCEL: break;
        }
        NFD_Quit();
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
        NFD_Init();

        nfdchar_t *scenePath = nullptr;
        nfdfilteritem_t filterItemLoadSce[1] = {{"Scene", "json"}};
        auto resultLoadSce = NFD_OpenDialog(&scenePath, filterItemLoadSce, 1, NULL);
        loading_result.clear();

        switch (resultLoadSce) {
            case NFD_OKAY: {
                logger.info("Load Scene") << scenePath;
                ImGui::OpenPopup("Load");
                try {
                    engine.loadScene(scenePath);
                    loading_result = "Scene correctly loaded";
                } catch (const std::exception &e) {
                    logger.err() << e.what();
                    loading_result = e.what();
                    ImGui::OpenPopup("Load");
                }
                NFD_FreePath(scenePath);
            } break;
            case NFD_ERROR: {
                logger.err("File Dialog") << NFD_GetError();
            } break;
            case NFD_CANCEL: break;
        }
        NFD_Quit();
    }
    if (ImGui::BeginPopupModal("Load", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%s", loading_result.c_str());
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

void ImGuiManager::render() { ImGui::Render(); }
