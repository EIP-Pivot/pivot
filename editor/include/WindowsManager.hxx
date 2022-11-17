#pragma once

#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>
#include <pivot/engine.hxx>
#include <pivot/graphics/AssetStorage/AssetStorage.hxx>

#include "Windows/IWindow.hxx"
#include "Windows/SceneWindow.hxx"

namespace pivot::editor
{

class WindowsManager
{
public:
    struct Workspace {
        ImVec2 offset;
        ImVec2 size;
    };

public:
    WindowsManager(const ecs::component::Index &componentIndex, const pivot::ecs::systems::Index &systemIndex,
                   const ecs::SceneManager &sceneManager, ecs::CurrentScene scene,
                   pivot::graphics::AssetStorage &assetStorage, pivot::graphics::PipelineStorage &pipelineStorage,
                   Engine &engine, const builtins::Camera &camera, bool &paused);
    const ecs::component::Index &getComponentIndex();
    const ecs::systems::Index &getSystemIndex();
    ecs::CurrentScene getCurrentScene();
    pivot::graphics::AssetStorage &getAssetStorage();
    void setCurrentScene(ecs::SceneManager::SceneId sceneId);
    Entity getSelectedEntity() const;
    void setSelectedEntity(Entity entity);
    void render();

public:
    ImTextureID &getTextureId(const std::string &name);
    ImGuiID getCenterDockId() { return m_centerDockId; };
    void setAspectRatio(float aspect);
    void newFrame();
    void endFrame();
    void reset() { imguiTextureId.clear(); }

private:
    void dockSpace();

public:
    Workspace workspace;

private:
    std::map<std::string, std::unique_ptr<IWindow>> m_windows{};
    std::map<std::string, std::unique_ptr<IWindow>> m_scenes{};
    std::unordered_map<std::string, ImTextureID> imguiTextureId;
    Entity m_entitySelected{static_cast<Entity>(-1)};
    ImGuiID m_centerDockId{};
    ecs::SceneManager::SceneId m_sceneId;

private:
    const ecs::component::Index &m_componentIndex;
    const ecs::systems::Index &m_systemIndex;
    const ecs::SceneManager &m_sceneManager;
    ecs::CurrentScene m_scene;
    pivot::graphics::AssetStorage &m_assetStorage;
    pivot::graphics::PipelineStorage &m_pipelineStorage;
    pivot::Engine &m_engine;
    const builtins::Camera &m_camera;
    bool &m_paused;
};

}    // namespace pivot::editor