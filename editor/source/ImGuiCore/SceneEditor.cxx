#include "ImGuiCore/SceneEditor.hxx"

#include <ImGuizmo.h>

void SceneEditor::create()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowDockID(m_imGuiManager.getCenterDockId());
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove);
    ImGui::PopStyleVar();
    {
        ImGui::BeginChild("RenderViewport", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_NoBackground);

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        size = ImGui::GetContentRegionAvail();
        offset = ImGui::GetWindowPos();
        ImGui::EndChild();
    }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET")) {
            pivot_assert(payload->Data, "Empty drag and drop payload");
            auto *assetWrapper = reinterpret_cast<AssetBrowser::wrapper *>(payload->Data);
            assetWrapper->assetBrowser.createEntity(assetWrapper->name);
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::End();
}
