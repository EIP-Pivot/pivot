#pragma once

#include <pivot/engine.hxx>

#include <filesystem>
#include <functional>
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
        Success,
        HandlerError,
        Cancel,
        Error,
    };

    struct FileInteraction {
        const FileAction action;
        const std::string sButtonText;
        const std::string sSuccesText;
        const std::string sErrorText;
        const std::vector<nfdfilteritem_t> acceptedFiles;
        const std::function<bool(const std::filesystem::path &)> handler;

        FileResult open() const;
    };

public:
    MenuBar(const pivot::ecs::SceneManager &sceneManager, pivot::Engine &engine);

    bool render();

private:
    std::unordered_map<std::string, std::vector<FileInteraction>> fileSubMenu;
};
