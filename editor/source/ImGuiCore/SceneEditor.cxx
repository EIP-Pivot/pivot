#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>
#include <numbers>
#include <pivot/builtins/components/Transform.hxx>

#include "ImGuiCore/CustomWidget.hxx"
#include "ImGuiCore/SceneEditor.hxx"

void SceneEditor::create()
{
    ImGui::SetNextWindowDockID(m_imGuiManager.getCenterDockId());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    //    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
    ImGui::Begin(" Scene ", nullptr,
                 ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
    toolbar();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
    viewport();
    ImGui::End();
}

void SceneEditor::toolbar()
{
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.20f, 0.25f, 0.29f, 1.00f));
    if (ImGui::BeginMenuBar()) {
        imGuizmoOperation();
        imGuizmoMode();
        ImGui::EndMenuBar();
    }
    ImGui::PopStyleColor();
}

void SceneEditor::imGuizmoOperation()
{
    auto move = m_imGuiManager.getTextureId("MoveTool");
    auto rotate = m_imGuiManager.getTextureId("RotateTool");
    auto scale = m_imGuiManager.getTextureId("ScaleTool");

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
    if (CustomWidget::RadioImageButton("Translate", move, ImVec2(17.f, 17.f),
                                       currentGizmoOperation == ImGuizmo::TRANSLATE))
        currentGizmoOperation = ImGuizmo::TRANSLATE;
    if (CustomWidget::RadioImageButton("Rotate", rotate, ImVec2(17.f, 17.f), currentGizmoOperation == ImGuizmo::ROTATE))
        currentGizmoOperation = ImGuizmo::ROTATE;
    if (CustomWidget::RadioImageButton("Scale", scale, ImVec2(17.f, 17.f), currentGizmoOperation == ImGuizmo::SCALE))
        currentGizmoOperation = ImGuizmo::SCALE;
    ImGui::PopStyleVar(2);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5.f);
    ImGui::Separator();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5.f);
}

void SceneEditor::imGuizmoMode()
{
    if (currentGizmoOperation != ImGuizmo::SCALE) {
        auto move = m_imGuiManager.getTextureId("MoveTool");
        auto rotate = m_imGuiManager.getTextureId("RotateTool");

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
        if (CustomWidget::RadioImageButton("Local", move, ImVec2(17.f, 17.f), currentGizmoMode == ImGuizmo::LOCAL))
            currentGizmoMode = ImGuizmo::LOCAL;
        if (CustomWidget::RadioImageButton("World", rotate, ImVec2(17.f, 17.f), currentGizmoMode == ImGuizmo::WORLD))
            currentGizmoMode = ImGuizmo::WORLD;
        ImGui::PopStyleVar(2);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5.f);
        ImGui::Separator();
    } else {
        currentGizmoMode = ImGuizmo::LOCAL;
    }
}

void SceneEditor::viewport()
{
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
}

void SceneEditor::setAspectRatio(float aspect) { aspectRatio = aspect; }

void SceneEditor::DisplayGuizmo(Entity entity, const pivot::builtins::Camera &camera)
{
    using Transform = pivot::builtins::components::Transform;

    const auto view = camera.getView();
    const auto projection = camera.getProjection(pivot::Engine::fov, aspectRatio);

    const float *view_ptr = glm::value_ptr(view);
    const float *projection_ptr = glm::value_ptr(projection);

    // TODO: Refactor this out, to compute only when the scene changes
    auto &cm = m_currentScene->getComponentManager();
    auto &array = cm.GetComponentArray(cm.GetComponentId(Transform::description.name).value());
    auto &ro_array = dynamic_cast<pivot::ecs::component::DenseTypedComponentArray<pivot::graphics::Transform> &>(array);
    if (!ro_array.entityHasValue(entity)) return;
    pivot::graphics::Transform &transform = ro_array.getData()[entity];
    auto matrix = transform.getModelMatrix();
    float *matrix_data = glm::value_ptr(matrix);
    ImGuizmo::SetRect(offset.x, offset.y, size.x, size.y);
    glm::mat4x4 delta_matrix;
    bool changed = ImGuizmo::Manipulate(view_ptr, projection_ptr, currentGizmoOperation, currentGizmoMode, matrix_data,
                                        glm::value_ptr(delta_matrix), useSnap ? &snap[0] : NULL);
    if (!changed) return;

    glm::vec3 rotation_deg;
    ImGuizmo::DecomposeMatrixToComponents(matrix_data, glm::value_ptr(transform.position), glm::value_ptr(rotation_deg),
                                          glm::value_ptr(transform.scale));
    transform.rotation = rotation_deg * std::numbers::pi_v<float> / 180.0f;
}
