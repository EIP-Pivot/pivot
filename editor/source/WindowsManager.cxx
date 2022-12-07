#include "WindowsManager.hxx"

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui_internal.h>

#include "Windows/AssetWindow.hxx"
#include "Windows/ComponentWindow.hxx"
#include "Windows/EntityWindow.hxx"
#include "Windows/SceneWindow.hxx"
#include "Windows/SystemWindow.hxx"

using namespace pivot::editor;

WindowsManager::WindowsManager(const pivot::ecs::component::Index &componentIndex,
                               const pivot::ecs::systems::Index &systemIndex, const ecs::SceneManager &sceneManager,
                               pivot::ecs::CurrentScene scene, pivot::graphics::AssetStorage &assetStorage,
                               pivot::graphics::PipelineStorage &pipelineStorage, Engine &engine, bool &paused)
    : m_componentIndex(componentIndex),
      m_systemIndex(systemIndex),
      m_sceneManager(sceneManager),
      m_scene(scene),
      m_assetStorage(assetStorage),
      m_pipelineStorage(pipelineStorage),
      m_engine(engine),
      m_paused(paused)
{
    m_windows["ComponentWindow"] = std::make_unique<ComponentWindow>(*this);
    m_windows["SystemWindow"] = std::make_unique<SystemWindow>(*this);
    m_windows["EntityWindow"] = std::make_unique<EntityWindow>(*this);
    m_windows["AssetWindow"] = std::make_unique<AssetWindow>(*this);
}

const pivot::ecs::component::Index &WindowsManager::getComponentIndex() const { return m_componentIndex; }

const pivot::ecs::systems::Index &WindowsManager::getSystemIndex() const { return m_systemIndex; }

const pivot::Engine &WindowsManager::getEngine() const { return m_engine; }

pivot::Engine &WindowsManager::getEngine() { return m_engine; }

pivot::ecs::CurrentScene WindowsManager::getCurrentScene() { return m_scene; }

pivot::graphics::AssetStorage &WindowsManager::getAssetStorage() { return m_assetStorage; }

void WindowsManager::setCurrentScene(ecs::SceneManager::SceneId sceneId)
{
    if (m_sceneManager.getCurrentSceneId() != sceneId) {
        setSelectedEntity(-1);
        m_engine.changeCurrentScene(sceneId);
    }
}

ImTextureID &WindowsManager::getTextureId(const std::string &name)
{
    if (auto iter = imguiTextureId.find(name); iter == imguiTextureId.end()) {
        auto image = m_engine.getTexture(name);
        vk::Sampler sampler = m_engine.getSampler();
        imguiTextureId[name] = ImGui_ImplVulkan_AddTexture(sampler, image.imageView, (VkImageLayout)image.imageLayout);
        return imguiTextureId.at(name);
    } else {
        return iter->second;
    }
}

Entity WindowsManager::getSelectedEntity() const { return m_entitySelected; }

void WindowsManager::setSelectedEntity(Entity entity) { m_entitySelected = entity; }

void WindowsManager::render()
{
    if (ImGui::IsKeyPressed(ImGuiKey_N)) { m_engine.registerScene(); }
    if (ImGui::IsKeyPressed(ImGuiKey_W)) {
        ImGui::SetWindowFocus(std::string(" " + m_sceneManager.getSceneById(0).getName() + " ").c_str());
        m_scenes.erase(m_sceneManager.getCurrentScene().getName());
        m_engine.unregisterScene(m_scene.id());
    }
    for (ecs::SceneManager::SceneId sceneId = 0; sceneId < m_sceneManager.getLivingScene(); sceneId++) {
        const std::string &name = m_sceneManager.getSceneById(sceneId).getName();
        if (m_scenes.find(name) == m_scenes.end())
            m_scenes[name] = std::make_unique<SceneWindow>(sceneId, *this, m_pipelineStorage, m_paused);
    }
    for (auto &[_, window]: m_scenes) { window->render(); }
    for (auto &[_, window]: m_windows) {
        if (window->isOpen()) window->render();
    }
}
void WindowsManager::newFrame()
{
    PROFILE_FUNCTION();
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    dockSpace();
}

void WindowsManager::dockSpace()
{
    PROFILE_FUNCTION();
    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode |
                                             ImGuiDockNodeFlags_NoDockingInCentralNode |
                                             ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton;
        ImGuiWindowFlags host_window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                             ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                             ImGuiWindowFlags_NoNavFocus;
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            host_window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Window", nullptr, host_window_flags);
        {
            ImGui::PopStyleVar(3);

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

            ImGuiID dockspace_id = ImGui::GetID("DockSpace");
            ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2{0.f, 0.f});
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 7.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags, nullptr);
            ImGui::PopStyleVar(3);
            if (ImGuiDockNode *node = ImGui::DockBuilderGetCentralNode(dockspace_id)) {
                m_centerDockId = node->ID;
                if (node->TabBar) {
                    bool open = false;
                    ImGui::TabItemEx(node->TabBar, "X", &open, ImGuiTabItemFlags_Button, nullptr);
                    ImGui::TabItemEx(node->TabBar, "Trailing", &open, ImGuiTabItemFlags_Button, nullptr);
                }
            }
            ImGui::PopStyleColor();
        }
    }
    ImGui::End();
}

void WindowsManager::endFrame()
{
    PROFILE_FUNCTION();
    ImGui::EndFrame();
    ImGui::Render();
}

void WindowsManager::setAspectRatio(float aspect)
{
    for (auto &[_, scene]: m_scenes) dynamic_cast<SceneWindow *>(scene.get())->setAspectRatio(aspect);
}
void WindowsManager::resetScene(pivot::ecs::SceneManager::SceneId id, const nlohmann::json &json)
{
    m_engine.resetScene(id, json);
}

const pivot::ecs::Scene &WindowsManager::getSceneByID(pivot::ecs::SceneManager::SceneId id)
{
    return m_sceneManager.getSceneById(id);
}

void WindowsManager::setDefaultCamera() { m_engine.setCurrentCamera(std::nullopt); }
