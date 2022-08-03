#include "ImGuiCore/SceneEditor.hxx"

void SceneEditor::create()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Scene");
    ImGui::PopStyleVar();
    {
        ImGui::BeginChild("RenderViewport", ImVec2(0, 0));
        ImGui::EndChild();
    }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET")) {
            pivot_assert(payload->Data, "Empty drag and drop payload");
            auto *assetWrapper = reinterpret_cast<AssetBrowser::wrapper *>(payload->Data);
            assetWrapper->assetBrowser.createEntity(assetWrapper->asset_name);
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::End();
}
