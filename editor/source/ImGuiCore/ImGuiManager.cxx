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
    loadScript(engine);
    loadAsset(engine);
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

void ImGuiManager::loadScript(pivot::Engine &engine)
{
    if (ImGui::Button("Load Script")) {
        NFD::Guard nfd_guard;
        NFD::UniquePath scriptPath;
        nfdfilteritem_t filterItemScript[] = {{"PivotScript", "pivotscript"}};
        auto resultSave = NFD::OpenDialog(scriptPath, filterItemScript, 1);

        switch (resultSave) {
            case NFD_OKAY: {
                logger.info("Load Script") << scriptPath;
                try {
                    engine.loadScript(scriptPath.get());
                    ImGui::OpenPopup("LoadScriptOk");
                } catch (const std::exception &e) {
                    logger.err() << e.what();
                    ImGui::OpenPopup("LoadScriptFail");
                }
            } break;
            case NFD_ERROR: {
                logger.err("File Dialog") << NFD::GetError();
                NFD::ClearError();
                ImGui::OpenPopup("LoadScriptFail");
            } break;
            case NFD_CANCEL: break;
        }
    }
    if (ImGui::BeginPopupModal("LoadScriptOk", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Script loaded succefully !");
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("LoadScriptFail", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Script loading failed, please look at the logs.");
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

void ImGuiManager::loadAsset(pivot::Engine &engine)
{
    if (ImGui::Button("Load asset")) {
        NFD::Guard nfd_guard;
        NFD::UniquePath path;
        nfdfilteritem_t filterItem[] = {{"Model", "gltf"}, {"Model", "obj"}};

        switch (NFD::OpenDialog(path, filterItem, 2)) {
            case NFD_OKAY: {
                logger.info("Load asset") << path;
                engine.loadAsset(path.get());
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

void LoadingIndicatorCircle(const char *label, const float size)
{
    using namespace ImGui;
    const float speed = 6;
    const int circle_count = 10;
    const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);

    ImGuiWindow *window = GetCurrentWindow();
    if (window->SkipItems) { return; }

    ImGuiContext &g = *GImGui;
    const ImGuiID id = window->GetID(label);

    const ImVec2 pos = window->DC.CursorPos;
    const float circle_radius = size / 10.0f;
    const ImRect bb(pos, ImVec2(pos.x + size * 2.0f, pos.y + size * 2.0f));
    const ImGuiStyle &style = g.Style;
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id)) return;

    // Render
    const float t = g.Time;
    const auto degree_offset = 2.0f * IM_PI / circle_count;

    for (int i = 0; i < circle_count; ++i) {
        const auto x = size * std::sin(degree_offset * i);
        const auto y = size * std::cos(degree_offset * i);
        const auto growth = std::max(0.0f, std::sin(t * speed - i * degree_offset));
        window->DrawList->AddCircleFilled(ImVec2(pos.x + size + x, pos.y + size - y),
                                          circle_radius + growth * circle_radius, GetColorU32(col));
    }
}

void ImGuiManager::render() { ImGui::Render(); }
