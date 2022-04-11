#pragma once

#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/Event/index.hxx>
#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>

#include <pivot/graphics/VulkanApplication.hxx>

namespace pivot
{
class Engine
{
public:
    Engine();

    void run();

    void changeCurrentScene(ecs::SceneManager::SceneId sceneId);
    ecs::SceneManager::SceneId registerScene() { return m_scene_manager.registerScene(); }
    ecs::SceneManager::SceneId registerScene(std::string name) { return m_scene_manager.registerScene(name); }

protected:
    pivot::ecs::component::Index m_component_index;
    pivot::ecs::event::Index m_event_index;
    pivot::ecs::systems::Index m_system_index;
    pivot::graphics::VulkanApplication m_vulkan_application;
    Camera m_camera;
    bool m_paused = true;

#ifdef CULLING_DEBUG
    Camera m_culling_camera;
#endif

    virtual void onTick(float delta){};
    ecs::CurrentScene getCurrentScene() { return m_scene_manager; };
    const ecs::SceneManager &getSceneManager() { return m_scene_manager; };

private:
    pivot::ecs::SceneManager m_scene_manager;
};
}    // namespace pivot
