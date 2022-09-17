#pragma once

#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/engine.hxx>

#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>
#include <nfd.hpp>

template <class...>
constexpr std::false_type always_false;

class ImGuiManager
{

public:
    ImGuiManager(const pivot::ecs::SceneManager &sceneManager, pivot::Engine &engine, const std::filesystem::path &asset_dir)
        : m_sceneManager(sceneManager), m_engine(engine)
    {
        for (const auto &directoryEntry: std::filesystem::recursive_directory_iterator(asset_dir / "Editor")) {
            if (directoryEntry.is_directory()) continue;
            m_engine.loadAsset(directoryEntry.path(), false);
        }
    };

    void reset() { imguiTextureId.clear(); }
    void newFrame();
    void dockSpace();
    static void render();
    ImGuiID getCenterDockId();
    ImTextureID &getTextureId(const std::string &name)
    {
        if (auto iter = imguiTextureId.find(name); iter == imguiTextureId.end()) {
            auto image = m_engine.getTexture(name);
            vk::Sampler sampler = m_engine.getSampler();
            pivot::graphics::AllocatedImage texture = m_engine.getTexture(name);
            ImVec2 size(texture.size.width, texture.size.height);
            imguiTextureId[name] =
                ImGui_ImplVulkan_AddTexture(sampler, image.imageView, (VkImageLayout)image.imageLayout);
            return imguiTextureId.at(name);
        } else {
            return iter->second;
        }
    }

private:
    std::unordered_map<std::string, ImTextureID> imguiTextureId;
    const pivot::ecs::SceneManager &m_sceneManager;
    pivot::Engine &m_engine;
    ImGuiID m_centerDockId{};
};
