#pragma once

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>

#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/engine.hxx>
#include <pivot/graphics/VulkanApplication.hxx>

class Editor
{
public:
    Editor(const pivot::ecs::SceneManager &sceneManager, pivot::ecs::CurrentScene scene)
        : m_sceneManager(sceneManager), m_currentScene(scene), run(false){};
    void create(pivot::Engine &engine, pivot::graphics::PipelineStorage &pipelineStorage);
    pivot::ecs::SceneManager::SceneId addScene(pivot::Engine &engine);
    pivot::ecs::SceneManager::SceneId addScene(pivot::Engine &engine, std::string name);

    bool getRun();
    void setRun(bool run) { this->run = run; }

private:
    void createPopUp(pivot::Engine &engine);

    const pivot::ecs::SceneManager &m_sceneManager;
    pivot::ecs::CurrentScene m_currentScene;
    bool shouldForce = false;

    bool run;
    std::vector<std::string> availableModes;
};
