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
    enum class FileAction {
        Open,
        Save,
    };

    enum class MenuBarAction {
        None,
        SaveScene,
        LoadScene,
        LoadScript,
        LoadAsset,
    };

public:
    ImGuiManager(const pivot::ecs::SceneManager &sceneManager, pivot::Engine &engine)
        : m_sceneManager(sceneManager), m_engine(engine), m_centerDockId(0)
    {
        for (const auto &directoryEntry: std::filesystem::recursive_directory_iterator(PIVOT_EDITOR_ICON_PATH)) {
            if (directoryEntry.is_directory()) continue;
            m_engine.loadAsset(directoryEntry.path(), false);
        }
    };

    void reset() { imguiTextureId.clear(); }
    void setStyle();
    void newFrame();
    void dockSpace();
    bool menuBar();
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

    template <FileAction A>
    bool handleFile(const std::string &buttonText, const std::string &successText, const std::string &errorText,
                    const std::vector<nfdfilteritem_t> &acceptedFiles,
                    const std::function<bool(const std::filesystem::path &)> &&handler)
    {

        bool result_handler = false;

        NFD::Guard nfd_guard;
        NFD::UniquePath path;
        nfdresult_t result = nfdresult_t::NFD_ERROR;

        if constexpr (A == FileAction::Save) {
            result = NFD::SaveDialog(path, acceptedFiles.data(), acceptedFiles.size());
        } else if constexpr (A == FileAction::Open) {
            result = NFD::OpenDialog(path, acceptedFiles.data(), acceptedFiles.size());
        } else {
            static_assert(always_false<decltype(A)>, "Unreachable branch");
        }

        switch (result) {
            case NFD_OKAY: {
                logger.info(buttonText) << path;
                handler(path.get());
            } break;
            case NFD_ERROR: {
                logger.err("File Dialog") << NFD::GetError();
                NFD::ClearError();
            } break;
            case NFD_CANCEL: break;
        }

        return result_handler;
    }

private:
    std::unordered_map<std::string, ImTextureID> imguiTextureId;
    const pivot::ecs::SceneManager &m_sceneManager;
    pivot::Engine &m_engine;
    ImGuiID m_centerDockId{};
};
