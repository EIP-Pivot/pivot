#pragma once

#include <pivot/engine.hxx>

#include <filesystem>
#include <functional>
#include <imgui.h>
#include <nfd.hpp>
#include <unordered_map>
#include <vector>

class MenuBar
{
public:
    enum class FileAction {
        Open,
        Save,
    };

    enum class FileResult {
        ResetFrame = 2,
        Cancel = 1,
        Success = 0,
        HandlerError = -1,
        Error = -2,
    };

    using Handler = std::function<FileResult(const std::filesystem::path &)>;

    struct FileInteraction {
        const FileAction action;
        const std::string sButtonText;
        const std::string sSuccesText;
        const std::string sErrorText;
        const ImGuiKey_ key;
        const ImGuiKey_ mod;
        const std::vector<nfdfilteritem_t> acceptedFiles;
        Handler handler;
        FileResult open() const;
    };

public:
    MenuBar(const pivot::ecs::SceneManager &sceneManager, pivot::Engine &engine);

    bool render();

    bool shouldDisplayColorwindow() const { return displayColors; }

    bool shouldCaptureFrame() noexcept
    {
        bool temp = captureNextFrame;
        captureNextFrame = false;
        return temp;
    }

private:
    std::unordered_map<std::string, std::vector<FileInteraction>> fileSubMenu;

    bool showMetricsWindow = false;
    bool displayColors = false;
    bool captureNextFrame = false;
};
