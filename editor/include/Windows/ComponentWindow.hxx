#pragma once

#include "Windows/IWindow.hxx"
#include "WindowsManager.hxx"

#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/SceneManager.hxx>

namespace pivot::editor
{

class ComponentWindow : public IWindow
{
public:
    explicit ComponentWindow(WindowsManager &manager): IWindow(manager, true){};
    void render() override;

private:
    void addComponent(const ecs::component::Description &description);
    void displayComponent();
    void displayName();
    void deleteComponent(ecs::component::ComponentRef ref);
    void createPopUp();
    void selectCamera(pivot::ecs::component::ComponentRef ref);
};

}    // namespace pivot::editor
