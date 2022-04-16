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
    void saveScene(ecs::SceneManager::SceneId id, const std::filesystem::path &path);

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
    ecs::SceneManager m_scene_manager;
    OptionalRef<ecs::component::DenseTypedComponentArray<builtins::components::RenderObject>>
        m_current_scene_render_object;
};
}    // namespace pivot
