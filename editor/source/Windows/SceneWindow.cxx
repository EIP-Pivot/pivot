#include "Windows/SceneWindow.hxx"

#include <glm/gtc/type_ptr.hpp>
#include <numbers>

#include <pivot/graphics/types/Transform.hxx>

#include "ImGuiCore/CustomWidget.hxx"
#include "Windows/AssetWindow.hxx"

using namespace pivot::editor;

void SceneWindow::render()
{
    PROFILE_FUNCTION();
    ImGui::SetNextWindowDockID(m_manager.getCenterDockId());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
    if (ImGui::Begin(std::string(" " + m_manager.getSceneByID(m_sceneId).getName() + " ").c_str(), nullptr,
                     ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar)) {
        m_manager.setCurrentScene(m_sceneId);
    }
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
    toolbar();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
    viewport();
    if (m_manager.getSelectedEntity() != -1u) displayGuizmo();
    ImGui::End();
}

void SceneWindow::toolbar()
{
    PROFILE_FUNCTION();
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.20f, 0.25f, 0.29f, 1.00f));
    if (ImGui::BeginMenuBar()) {
        ImGui::Columns(3);
        imGuizmoOperation();
        imGuizmoMode();
        ImGui::NextColumn();
        setSceneStatus();
        ImGui::NextColumn();
        setPipelineStatus();
        ImGui::Columns();
        ImGui::EndMenuBar();
    }
    ImGui::PopStyleColor();
}

void SceneWindow::imGuizmoOperation()
{
    PROFILE_FUNCTION();
    auto camera = m_manager.getTextureId("Camera");
    auto move = m_manager.getTextureId("MoveTool");
    auto rotate = m_manager.getTextureId("RotateTool");
    auto scale = m_manager.getTextureId("ScaleTool");

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
    if (ImGui::ImageButton("Camera", camera, ImVec2(17.f, 17.f))) {
        m_manager.setDefaultCamera();
    }
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5.f);
    ImGui::Separator();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5.f);
    if (CustomWidget::RadioImageButton("Translate", move, ImVec2(17.f, 17.f),
                                       currentGizmoOperation == ImGuizmo::TRANSLATE))
        currentGizmoOperation = ImGuizmo::TRANSLATE;
    if (CustomWidget::RadioImageButton("Rotate", rotate, ImVec2(17.f, 17.f), currentGizmoOperation == ImGuizmo::ROTATE))
        currentGizmoOperation = ImGuizmo::ROTATE;
    if (CustomWidget::RadioImageButton("Scale", scale, ImVec2(17.f, 17.f), currentGizmoOperation == ImGuizmo::SCALE))
        currentGizmoOperation = ImGuizmo::SCALE;
    ImGui::PopStyleVar(2);
}

void SceneWindow::imGuizmoMode()
{
    PROFILE_FUNCTION();
    if (currentGizmoOperation != ImGuizmo::SCALE) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5.f);
        ImGui::Separator();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5.f);
        auto move = m_manager.getTextureId("MoveTool");
        auto rotate = m_manager.getTextureId("RotateTool");

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
        if (CustomWidget::RadioImageButton("Local", move, ImVec2(17.f, 17.f), currentGizmoMode == ImGuizmo::LOCAL))
            currentGizmoMode = ImGuizmo::LOCAL;
        if (CustomWidget::RadioImageButton("World", rotate, ImVec2(17.f, 17.f), currentGizmoMode == ImGuizmo::WORLD))
            currentGizmoMode = ImGuizmo::WORLD;
        ImGui::PopStyleVar(2);
    } else {
        currentGizmoMode = ImGuizmo::LOCAL;
    }
}

void SceneWindow::setSceneStatus()
{
    PROFILE_FUNCTION();
    auto play = m_manager.getTextureId("Play");
    auto pause = m_manager.getTextureId("Pause");
    auto stop = m_manager.getTextureId("Stop");

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
    if (sceneStatus == SceneStatus::STOP)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() / 2) - 34.f);
    else
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() / 2) - 51.f);
    if (CustomWidget::RadioImageButton("Play", play, ImVec2(17.f, 17.f), sceneStatus == SceneStatus::PLAY)) {
        if (sceneStatus == SceneStatus::STOP) {
            m_save = m_manager.getCurrentScene()->getJson();
        }
        sceneStatus = SceneStatus::PLAY;
        m_paused = false;
    }
    if (sceneStatus != SceneStatus::STOP)
        if (CustomWidget::RadioImageButton("Pause", pause, ImVec2(17.f, 17.f), sceneStatus == SceneStatus::PAUSE)) {
            sceneStatus = SceneStatus::PAUSE;
            m_paused = true;
        }
    if (CustomWidget::RadioImageButton("Stop", stop, ImVec2(17.f, 17.f), sceneStatus == SceneStatus::STOP)) {
        if (sceneStatus != SceneStatus::STOP) {
            m_manager.resetScene(m_manager.getCurrentScene().id(), m_save);
        }
        sceneStatus = SceneStatus::STOP;
        m_paused = true;
    }
    ImGui::PopStyleVar(2);
}

void SceneWindow::setPipelineStatus()
{
    PROFILE_FUNCTION();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);

    ImGui::PushItemWidth(ImGui::GetColumnWidth() / 3);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() / 3) * 2);
    if (ImGui::BeginCombo("##pipeline", m_pipelineStorage.getDefaultName().c_str())) {
        for (const auto &msaa: m_pipelineStorage.getNames()) {
            bool is_selected = (m_pipelineStorage.getDefaultName() == msaa);
            if (ImGui::Selectable(msaa.c_str(), is_selected)) { m_pipelineStorage.setDefault(msaa); }
            if (is_selected) { ImGui::SetItemDefaultFocus(); }
        }
        ImGui::EndCombo();
    }
    if (ImGui::BeginCombo("##sample_count", "oui")) {

        for (const auto &word: {"oui", "non", "Why not ?"}) {
            if (ImGui::Selectable(word)) { std::cout << word << std::endl; }
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleVar(2);
}

void SceneWindow::viewport()
{
    PROFILE_FUNCTION();
    {
        ImGui::BeginChild("RenderViewport", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_NoBackground);

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        m_manager.workspace.size = ImGui::GetContentRegionAvail();
        m_manager.workspace.offset = ImGui::GetWindowPos();
        ImGui::EndChild();
    }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET")) {
            pivotAssertMsg(payload->Data, "Empty drag and drop payload");
            auto *assetWrapper = reinterpret_cast<AssetWindow::wrapper *>(payload->Data);
            assetWrapper->assetBrowser.createEntity(assetWrapper->name);
        }
        ImGui::EndDragDropTarget();
    }
}

void SceneWindow::setAspectRatio(float aspect) { aspectRatio = aspect; }

void SceneWindow::displayGuizmo()
{
    using Transform = pivot::graphics::Transform;

    PROFILE_FUNCTION();

    pivot::Entity entity = m_manager.getSelectedEntity();

    const glm::mat4 view = m_manager.getEngine().getCurrentCamera().getView();
    const glm::mat4 projection =
        m_manager.getEngine().getCurrentCamera().getProjection(pivot::Engine::fov, aspectRatio);

    const float *view_ptr = glm::value_ptr(view);
    const float *projection_ptr = glm::value_ptr(projection);

    // TODO: Refactor this out, to compute only when the scene changes
    auto &cm = m_manager.getCurrentScene()->getComponentManager();
    auto &array = cm.GetComponentArray(cm.GetComponentId(Transform::description.name).value());
    auto &transform_array = dynamic_cast<pivot::graphics::SynchronizedTransformArray &>(array);
    if (!transform_array.entityHasValue(entity)) return;

    auto transform_lock = transform_array.lock();
    pivot::graphics::Transform &transform = transform_array.getData()[entity];
    auto matrix = transform.getModelMatrix();
    float *matrix_data = glm::value_ptr(matrix);
    ImGuizmo::SetRect(m_manager.workspace.offset.x, m_manager.workspace.offset.y, m_manager.workspace.size.x,
                      m_manager.workspace.size.y);
    glm::mat4x4 delta_matrix;
    bool changed = ImGuizmo::Manipulate(view_ptr, projection_ptr, currentGizmoOperation, currentGizmoMode, matrix_data,
                                        glm::value_ptr(delta_matrix), useSnap ? &snap[0] : NULL);
    if (!changed) return;

    glm::vec3 rotation_deg;
    ImGuizmo::DecomposeMatrixToComponents(matrix_data, glm::value_ptr(transform.position), glm::value_ptr(rotation_deg),
                                          glm::value_ptr(transform.scale));
    transform.rotation = rotation_deg * std::numbers::pi_v<float> / 180.0f;
}
