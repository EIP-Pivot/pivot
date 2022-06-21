#pragma once

#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/engine.hxx>

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

public:
    ImGuiManager(const pivot::ecs::SceneManager &sceneManager): m_sceneManager(sceneManager){};
    void newFrame(pivot::Engine &engine);
    void render();

    template <FileAction A>
    bool handleFile(const std::string &buttonText, const std::string &successText, const std::string &errorText,
                    const std::vector<nfdfilteritem_t> &acceptedFiles,
                    const std::function<bool(const std::filesystem::path &)> &&handler)
    {
        std::string popupFailed = buttonText + "Failed";
        std::string popupSuccess = buttonText + "Success";

        bool result_handler = false;

        if (ImGui::Button(buttonText.c_str())) {
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
                    if (!handler(path.get())) ImGui::OpenPopup(popupFailed.c_str());
                    ImGui::OpenPopup(popupSuccess.c_str());
                } break;
                case NFD_ERROR: {
                    logger.err("File Dialog") << NFD::GetError();
                    NFD::ClearError();
                    ImGui::OpenPopup(popupFailed.c_str());
                } break;
                case NFD_CANCEL: break;
            }
        }

        if (ImGui::BeginPopupModal(popupSuccess.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text(successText.c_str());
            if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
            result_handler = true;
        }
        if (ImGui::BeginPopupModal(popupFailed.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text(errorText.c_str());
            if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
            result_handler = false;
        }
        return result_handler;
    }

private:
    const pivot::ecs::SceneManager &m_sceneManager;
};
