#pragma once

#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/Event/index.hxx>
#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>

#include <pivot/graphics/VulkanApplication.hxx>

#include <pivot/script/Engine.hxx>

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
    ecs::SceneManager::SceneId registerScene(std::unique_ptr<ecs::Scene> scene);
    void saveScene(ecs::SceneManager::SceneId id, const std::filesystem::path &path);
    ecs::SceneManager::SceneId loadScene(const std::filesystem::path &path);

    void loadScript(const std::filesystem::path &path);
    void loadAsset(const std::filesystem::path &path);

    static constexpr float fov = 80;

protected:
    ecs::component::Index m_component_index;
    ecs::event::Index m_event_index;
    ecs::systems::Index m_system_index;
    graphics::VulkanApplication m_vulkan_application;
    ecs::script::Engine m_scripting_engine;
    builtins::Camera m_camera;
    bool m_paused = true;

    virtual void onTick(float delta){};
    ecs::CurrentScene getCurrentScene() { return m_scene_manager; };
    const ecs::SceneManager &getSceneManager() { return m_scene_manager; };

private:
    ecs::SceneManager m_scene_manager;
    OptionalRef<ecs::component::DenseTypedComponentArray<builtins::components::RenderObject>>
        m_current_scene_render_object;
};
}    // namespace pivot
