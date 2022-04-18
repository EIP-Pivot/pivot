#include "ImGuiCore/ImGuiManager.hxx"

#include <nfd.h>
#include <stdio.h>
#include <stdlib.h>
#include <Logger.hpp>

void ImGuiManager::newFrame(pivot::Engine &engine)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    if (ImGui::Button("Save Scene")){
        NFD_Init();

        nfdchar_t* savePath;
        nfdfilteritem_t filterItemSave[1] = {{"Scene", "json"}};
        std::string filename = m_sceneManager.getCurrentScene().getName() + ".json";
        nfdresult_t resultSave = NFD_SaveDialog(&savePath, filterItemSave, 1, NULL, filename.data());

        if (resultSave == NFD_OKAY) {
            logger.info("File Dialog") << savePath;
            m_sceneManager.getCurrentScene().save(savePath);
            ImGui::OpenPopup("Save");
        } else if (resultSave != NFD_CANCEL){
            logger.err("File Dialog") << NFD_GetError();
        }

        NFD_Quit();
    }

    if (ImGui::BeginPopupModal("Save", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Scene correctly saved");
        if (ImGui::Button("OK", ImVec2(120,0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    if (ImGui::Button("Load Scene")){
        NFD_Init();

        nfdchar_t *scenePath;
        nfdfilteritem_t filterItemLoadSce[1] = {{ "Scene", "json" }};
        nfdresult_t resultLoadSce = NFD_OpenDialog(&scenePath, filterItemLoadSce, 1, NULL);

        if (resultLoadSce == NFD_OKAY){
            logger.info("File Dialog") << scenePath;
            ImGui::OpenPopup("Load");
            try{
                engine.loadScene(scenePath);
                loading_result = "Scene correctly loaded";
            } catch (const std::exception& e){
                loading_result = std::string(e.what());
                ImGui::OpenPopup("Load");
            }
            NFD_FreePath(scenePath);
        } else if (resultLoadSce != NFD_CANCEL){
            logger.err("File Dialog") << NFD_GetError();
        }


        NFD_Quit();
    }
    if (ImGui::BeginPopupModal("Load", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", loading_result.c_str());
        if (ImGui::Button("OK", ImVec2(120,0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

void ImGuiManager::render() { ImGui::Render(); }
