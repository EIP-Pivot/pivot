#pragma once

#include "Windows/IWindow.hxx"
#include "WindowsManager.hxx"

namespace pivot::editor
{

class AssetWindow : public IWindow
{
public:
    struct wrapper {
        AssetWindow &assetBrowser;
        char name[255];
    };

    AssetWindow(WindowsManager &manager): IWindow(manager, true){};
    void render() override;
    void createEntity(std::string_view name);

private:
    void dropSource(const std::string &name);
};

}    // namespace pivot::editor