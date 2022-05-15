#include "ImGuiCore/ImGuiManager.hxx"

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <Logger.hpp>
#include <nfd.hpp>

void ImGuiManager::newFrame(pivot::Engine &engine, pivot::graphics::VulkanApplication &vulkanApplication)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    ImGui::Begin("Load/Save");
    saveScene(engine);
    loadScene(engine);
    ImGui::End();

    ImGui::Begin("Assets");
    loadAsset(vulkanApplication);
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
                ImGui::OpenPopup("SaveOk");
            } break;
            case NFD_ERROR: {
                logger.err("File Dialog") << NFD::GetError();
                NFD::ClearError();
                ImGui::OpenPopup("SaveFail");
            } break;
            case NFD_CANCEL: break;
        }
    }

    if (ImGui::BeginPopupModal("SaveOk", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Scene correctly saved.");
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("SaveFail", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to save the scene, please check the log.");
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

void ImGuiManager::loadScene(pivot::Engine &engine)
{
    if (ImGui::Button("Load Scene")) {
        NFD::Guard nfd_guard;
        NFD::UniquePath scenePath;
        nfdfilteritem_t filterItemSave[] = {{"Scene", "json"}};
        auto filename = m_sceneManager.getCurrentScene().getName() + ".json";
        auto resultSave = NFD::OpenDialog(scenePath, filterItemSave, 1);

        switch (resultSave) {
            case NFD_OKAY: {
                logger.info("Load Scene") << scenePath;
                try {
                    engine.loadScene(scenePath.get());
                    ImGui::OpenPopup("LoadOk");
                } catch (const std::exception &e) {
                    logger.err() << e.what();
                    ImGui::OpenPopup("LoadFail");
                }
            } break;
            case NFD_ERROR: {
                logger.err("File Dialog") << NFD::GetError();
                NFD::ClearError();
                ImGui::OpenPopup("LoadFail");
            } break;
            case NFD_CANCEL: break;
        }
    }
    if (ImGui::BeginPopupModal("LoadOk", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Scene loaded succefully !");
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("LoadFail", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Scene loading failed, please look at the logs.");
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

void ImGuiManager::render() { ImGui::Render(); }

void ImGuiManager::loadAsset(pivot::graphics::VulkanApplication &vulkanApplication)
{
    if (ImGui::Button("Load asset")) {
        NFD::Guard nfd_guard;
        NFD::UniquePath path;
        nfdfilteritem_t filterItem[] = {{"Model", "gltf"}, {"Model", "obj"}};

        switch (NFD::OpenDialog(path, filterItem, 2)) {
            case NFD_OKAY: {
                logger.info("Load asset") << path;
                vulkanApplication.assetStorage.addModel(path.get());
                vulkanApplication.buildAssetStorage(pivot::graphics::AssetStorage::BuildFlagBits::eReloadOldAssets);
                ImGui::OpenPopup("LoadAssetOK");
            } break;
            case NFD_ERROR: {
                logger.err("File Dialog") << NFD::GetError();
                NFD::ClearError();
                ImGui::OpenPopup("LoadFail");
            } break;
            case NFD_CANCEL: break;
        }
    }
    if (ImGui::BeginPopupModal("LoadAssetOK", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Asset loaded succefully !");
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("LoadFail", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Asset loading failed, please look at the logs.");
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}
