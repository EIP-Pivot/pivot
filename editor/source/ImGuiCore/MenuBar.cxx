#include "ImGuiCore/MenuBar.hxx"

#include <imgui.h>

MenuBar::MenuBar(const pivot::ecs::SceneManager &sceneManager, pivot::Engine &engine)
{
    PROFILE_FUNCTION();
    fileSubMenu = std::unordered_map<std::string, std::vector<FileInteraction>>{
        {
            "File",
            std::vector<FileInteraction>{
                FileInteraction{
                    .action = FileAction::Save,
                    .sButtonText = "Save Scene",
                    .sSuccesText = "Scene correctly saved.",
                    .sErrorText = "Failed to save the scene, please check the log.",
                    .acceptedFiles = {{"Scene", "json"}},
                    .handler =
                        [&](const std::filesystem::path &path) {
                            engine.saveScene(sceneManager.getCurrentSceneId(), path);
                            return FileResult::Success;
                        },
                },
                FileInteraction{
                    .action = FileAction::Open,
                    .sButtonText = "Load Scene",
                    .sSuccesText = "Scene loaded succefully !",
                    .sErrorText = "Scene loading failed, please check the log.",
                    .acceptedFiles = {{"Scene", "json"}},
                    .handler =
                        [&](const std::filesystem::path &path) {
                            engine.loadScene(path);
                            return FileResult::ResetFrame;
                        },
                },
                FileInteraction{
                    .action = FileAction::Open,
                    .sButtonText = "Load Script",
                    .sSuccesText = "Script loaded succefully !",
                    .sErrorText = "Script loading failed, please look at the logs.",
                    .acceptedFiles = {{"Pivot script", "pivotscript"}},
                    .handler =
                        [&](const std::filesystem::path &path) {
                            engine.loadScript(path);
                            return FileResult::Success;
                        },
                },
                FileInteraction{
                    .action = FileAction::Open,
                    .sButtonText = "Load asset",
                    .sSuccesText = "Asset loaded succefully !",
                    .sErrorText = "Asset loading failed, please look at the logs.",
                    .acceptedFiles = {{"Model", "gltf,obj"}, {"Textures", "jpg,png,ktx"}},
                    .handler =
                        [&](const std::filesystem::path &path) {
                            engine.loadAsset(path);
                            return FileResult::ResetFrame;
                        },
                },
            },
        },
    };
}

bool MenuBar::render()
{
    PROFILE_FUNCTION();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{10.0f, 10.0f});
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 5.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    if (ImGui::BeginMainMenuBar()) {
        for (const auto &[menu_name, menu_item]: fileSubMenu) {
            if (ImGui::BeginMenu(menu_name.c_str())) {
                for (const auto &fileInteraction: menu_item) {
                    if (ImGui::MenuItem(fileInteraction.sButtonText.c_str())) {
                        switch (fileInteraction.open()) {
                            case FileResult::HandlerError:
                            case FileResult::ResetFrame: return false;
                            case FileResult::Success:
                            case FileResult::Error:
                            case FileResult::Cancel: break;
                        }
                    }
                }
                ImGui::EndMenu();
            }
        }

        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem("Metrics Window", "", showMetricsWindow)) { showMetricsWindow = !showMetricsWindow; }
            if (ImGui::MenuItem("Color Window", "", displayColors)) { displayColors = !displayColors; }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (showMetricsWindow) ImGui::ShowMetricsWindow();

    ImGui::PopStyleVar(3);
    return true;
}

MenuBar::FileResult MenuBar::FileInteraction::open() const
{
    PROFILE_FUNCTION();
    NFD::Guard nfd_guard;
    NFD::UniquePath path;
    nfdresult_t result = nfdresult_t::NFD_ERROR;

    if (action == FileAction::Save) {
        result = NFD::SaveDialog(path, acceptedFiles.data(), acceptedFiles.size());
    } else if (action == FileAction::Open) {
        result = NFD::OpenDialog(path, acceptedFiles.data(), acceptedFiles.size());
    } else {
        pivotAssertMsg(false, "Unsupported file action !");
    }

    switch (result) {
        case NFD_OKAY: logger.info(sButtonText) << path; return handler(path.get());
        case NFD_ERROR:
            logger.err("File Dialog") << NFD::GetError();
            NFD::ClearError();
            return MenuBar::FileResult::Error;
        case NFD_CANCEL: return MenuBar::FileResult::Cancel;
    }
    return FileResult::Error;
}
