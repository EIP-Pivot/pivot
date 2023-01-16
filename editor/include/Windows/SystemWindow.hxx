#pragma once

#include "Windows/IWindow.hxx"
#include "WindowsManager.hxx"

namespace pivot::editor
{

class SystemWindow : public IWindow
{
public:
    explicit SystemWindow(WindowsManager &manager): IWindow(manager, true){};
    void render() override;

private:
    void displaySystem();
    void createPopUp();
};

}    // namespace pivot::editor