#pragma once

#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/Event/index.hxx>
#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>

#include <pivot/graphics/DrawCallResolver.hxx>
#include <pivot/graphics/VulkanApplication.hxx>
#include <pivot/graphics/types/AllocatedImage.hxx>

#include <pivot/script/Engine.hxx>

#include <pivot/builtins/components/RenderObject.hxx>
#include <pivot/internal/CameraArray.hxx>
#include <pivot/internal/LocationCamera.hxx>

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
    void loadAsset(const std::filesystem::path &path, bool reload = true);
    const graphics::AllocatedImage &getTexture(const std::string &name) const
    {
        return m_vulkan_application.assetStorage.get<graphics::AssetStorage::Texture>(name);
    }
    vk::Sampler getSampler() const { return m_vulkan_application.assetStorage.getSampler(); }

    void setCurrentCamera(std::optional<Entity> camera);
    internals::LocationCamera getCurrentCamera();

    static constexpr float fov = 80;

protected:
    ecs::component::Index m_component_index;
    ecs::event::Index m_event_index;
    ecs::systems::Index m_system_index;
    graphics::VulkanApplication m_vulkan_application;
    ecs::script::Engine m_scripting_engine;
    bool m_paused = true;
    std::optional<vk::Rect2D> renderArea = std::nullopt;

    virtual void onFrameStart() {}
    virtual void onTick([[maybe_unused]] float delta) {}
    virtual void onFrameEnd() {}
    virtual void onReset() {}
    void setRenderArea(vk::Rect2D renderArea) { this->renderArea = renderArea; }
    ecs::CurrentScene getCurrentScene() { return m_scene_manager; };
    const ecs::SceneManager &getSceneManager() { return m_scene_manager; };

private:
    ecs::SceneManager m_scene_manager;
    std::optional<graphics::DrawCallResolver::DrawSceneInformation> m_current_scene_draw_command;
    pivot::OptionalRef<internals::CameraArray> m_camera_array;
    pivot::OptionalRef<ecs::component::DenseTypedComponentArray<graphics::Transform>> m_transform_array;
    builtins::components::Camera m_default_camera_data;
    graphics::Transform m_default_camera_transform;

    bool isKeyPressed(const std::string &key) const;
    void onKeyPressed(graphics::Window &window, const graphics::Window::Key key, const graphics::Window::Modifier);

protected:
    internals::LocationCamera m_default_camera;
};
}    // namespace pivot
