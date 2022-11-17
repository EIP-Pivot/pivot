#pragma once

#include "Windows/IWindow.hxx"
#include "WindowsManager.hxx"

#include <pivot/ecs/Core/types.hxx>

namespace pivot::editor
{

class EntityWindow : public IWindow
{
public:
    explicit EntityWindow(WindowsManager &manager): IWindow(manager, true){};
    void render() override;

private:
    Entity addEntity();
    Entity addEntity(std::string name);
    void removeEntity();
    void createPopUp();
};

}    // namespace pivot::editor