#pragma once

#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/engine.hxx>

class ImGuiManager
{
public:
    ImGuiManager(const pivot::ecs::SceneManager &sceneManager): m_sceneManager(sceneManager){};
    void newFrame(pivot::Engine &engine, pivot::graphics::VulkanApplication &vulkanApplication);
    void saveScene(pivot::Engine &engine);
    void loadScene(pivot::Engine &engine);
    void loadAsset(pivot::graphics::VulkanApplication &vulkanApplication);
    void render();

private:
    const pivot::ecs::SceneManager &m_sceneManager;
};
