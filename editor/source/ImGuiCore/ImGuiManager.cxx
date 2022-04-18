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
            if (ImGui::BeginPopupModal("Save")){
                ImGui::Text("Scene correctly saved");
                ImGui::EndPopup();
            }
        } else if (resultSave != NFD_CANCEL){
            logger.err("File Dialog") << NFD_GetError();
        }

        NFD_Quit();
        }

    if (ImGui::Button("Load Scene")){
        NFD_Init();

        nfdchar_t *scenePath;
        nfdfilteritem_t filterItemLoadSce[1] = {{ "Scene", "json" }};
        nfdresult_t resultLoadSce = NFD_OpenDialog(&scenePath, filterItemLoadSce, 1, NULL);

        if (resultLoadSce == NFD_OKAY){
            logger.info("File Dialog") << scenePath;
            try{
                engine.loadScene(scenePath);
                if (ImGui::BeginPopupModal("Load")){
                    ImGui::Text("Scene correctly loaded");
                    ImGui::EndPopup();
                }
            } catch (const std::runtime_error& e){
                if (ImGui::BeginPopupModal("Load")){
                    ImGui::Text(e.what());
                    ImGui::EndPopup();
                }
            }
            NFD_FreePath(scenePath);
        } else if (resultLoadSce != NFD_CANCEL){
            logger.err("File Dialog") << NFD_GetError();
        }

        NFD_Quit();
    }
}

void ImGuiManager::render() { ImGui::Render(); }
