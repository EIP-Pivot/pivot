#pragma once

#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/Event/index.hxx>
#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>

#include <pivot/graphics/VulkanApplication.hxx>

#include <pivot/builtins/components/RenderObject.hxx>

namespace pivot
{
class Engine
{
public:
    Engine();

    void run();

    void changeCurrentScene(ecs::SceneManager::SceneId sceneId);
    ecs::SceneManager::SceneId registerScene();
    ecs::SceneManager::SceneId registerScene(std::string name);

    static constexpr float fov = 80;

protected:
    ecs::component::Index m_component_index;
    ecs::event::Index m_event_index;
    ecs::systems::Index m_system_index;
    graphics::VulkanApplication m_vulkan_application;
    builtins::Camera m_camera;
    bool m_paused = true;

#ifdef CULLING_DEBUG
    builtins::Camera m_culling_camera;
#endif

    virtual void onTick(float delta){};
    ecs::CurrentScene getCurrentScene() { return m_scene_manager; };
    const ecs::SceneManager &getSceneManager() { return m_scene_manager; };

private:
    ecs::SceneManager m_scene_manager;
    OptionalRef<ecs::component::DenseTypedComponentArray<builtins::components::RenderObject>>
        m_current_scene_render_object;
};
}    // namespace pivot
