#pragma once

#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/engine.hxx>
#include <pivot/script/Engine.hxx>

class ImGuiManager
{
public:
    ImGuiManager(const pivot::ecs::SceneManager &sceneManager, pivot::ecs::script::Engine scriptEngine): m_sceneManager(sceneManager),
                                                                                                               scriptEngine(scriptEngine){};
    void newFrame(pivot::Engine &engine);
    void saveScene(pivot::Engine &engine);
    void loadScene(pivot::Engine &engine);
    void loadScript();
    void render();

private:
    const pivot::ecs::SceneManager &m_sceneManager;
    pivot::ecs::script::Engine &scriptEngine;
};
