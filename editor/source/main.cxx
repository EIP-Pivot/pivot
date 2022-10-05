#include <iostream>

#include <pivot/graphics/Window.hxx>
#include <pivot/graphics/vk_utils.hxx>

#include <pivot/builtins/systems/ControlSystem.hxx>
#include <pivot/internal/camera.hxx>

#include <pivot/ecs/Core/Event/description.hxx>

#include <pivot/ecs/Core/Scene.hxx>
#include <pivot/ecs/Core/SceneManager.hxx>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "CmdLineArg.hxx"
#include "ImGuiCore/AssetBrowser.hxx"
#include "ImGuiCore/ComponentEditor.hxx"
#include "ImGuiCore/Editor.hxx"
#include "ImGuiCore/EntityModule.hxx"
#include "ImGuiCore/ImGuiManager.hxx"
#include "ImGuiCore/ImGuiTheme.hxx"
#include "ImGuiCore/MenuBar.hxx"
#include "ImGuiCore/SceneEditor.hxx"
#include "ImGuiCore/SystemsEditor.hxx"

#include <pivot/engine.hxx>
#include <pivot/utility/benchmark.hxx>

using namespace pivot::ecs;
using Window = pivot::graphics::Window;

class Application : public pivot::Engine
{
public:
    Application()
        : Engine(),
          imGuiManager(getSceneManager(), *this),
          editor(getSceneManager(), getCurrentScene()),
          entity(getCurrentScene()),
          componentEditor(m_component_index, getCurrentScene()),
          systemsEditor(m_system_index, m_component_index, getCurrentScene()),
          assetBrowser(imGuiManager, m_vulkan_application.assetStorage, getCurrentScene()),
          sceneEditor(imGuiManager, getCurrentScene()),
          menuBar(getSceneManager(), *this)
    {
    }

    SceneManager::SceneId loadDefaultScene() { return registerScene("Default"); }

    void init()
    {
        PROFILE_FUNCTION();
        auto &window = m_vulkan_application.window;

        Scene &scene = *getCurrentScene();

        window.addKeyReleaseCallback(Window::Key::LEFT_ALT,
                                     [&](Window &window, const Window::Key, const Window::Modifier) {
                                         window.captureCursor(!window.captureCursor());
                                         bFirstMouse = window.captureCursor();
                                         button.reset();
                                     });
        window.addKeyReleaseCallback(
            Window::Key::V, [&](Window &, const Window::Key, const Window::Modifier) { scene.switchCamera(); });

        auto key_lambda_press = [&](Window &window, const Window::Key key, const Window::Modifier) {
            if (window.captureCursor()) button.set(static_cast<std::size_t>(key));
        };
        auto key_lambda_release = [&](Window &window, const Window::Key key, const Window::Modifier) {
            if (window.captureCursor()) button.reset(static_cast<std::size_t>(key));
        };
        // Press action
        window.addKeyPressCallback(Window::Key::Z, key_lambda_press);
        window.addKeyPressCallback(Window::Key::Q, key_lambda_press);
        window.addKeyPressCallback(Window::Key::S, key_lambda_press);
        window.addKeyPressCallback(Window::Key::D, key_lambda_press);
        window.addKeyPressCallback(Window::Key::SPACE, key_lambda_press);
        window.addKeyPressCallback(Window::Key::LEFT_SHIFT, key_lambda_press);
        // Release action
        window.addKeyReleaseCallback(Window::Key::Z, key_lambda_release);
        window.addKeyReleaseCallback(Window::Key::Q, key_lambda_release);
        window.addKeyReleaseCallback(Window::Key::S, key_lambda_release);
        window.addKeyReleaseCallback(Window::Key::D, key_lambda_release);
        window.addKeyReleaseCallback(Window::Key::SPACE, key_lambda_release);
        window.addKeyReleaseCallback(Window::Key::LEFT_SHIFT, key_lambda_release);

        window.addMouseMovementCallback([&](Window &window, const glm::dvec2 pos) {
            if (!window.captureCursor()) return;

            if (bFirstMouse) {
                last = pos;
                bFirstMouse = false;
            }
            auto xoffset = pos.x - last.x;
            auto yoffset = last.y - pos.y;

            last = pos;
            pivot::builtins::systems::ControlSystem::processMouseMovement(m_camera, glm::dvec2(xoffset, yoffset));
        });
        m_vulkan_application.buildAssetStorage(pivot::graphics::AssetStorage::BuildFlagBits::eReloadOldAssets);
        // resize or loading asset reset imgui -> style reset
        //        ImGuiTheme::setStyle();
    }

    void processKeyboard(const pivot::builtins::Camera::Movement direction, float dt) noexcept
    {
        PROFILE_FUNCTION();
        using Camera = pivot::builtins::Camera;
        switch (direction) {
            case Camera::Movement::FORWARD: {
                m_camera.position.x += m_camera.front.x * 10.f * dt;
                m_camera.position.z += m_camera.front.z * 10.f * dt;
            } break;
            case Camera::Movement::BACKWARD: {
                m_camera.position.x -= m_camera.front.x * 10.f * dt;
                m_camera.position.z -= m_camera.front.z * 10.f * dt;
            } break;
            case Camera::Movement::RIGHT: {
                m_camera.position.x += m_camera.right.x * 10.f * dt;
                m_camera.position.z += m_camera.right.z * 10.f * dt;
            } break;
            case Camera::Movement::LEFT: {
                m_camera.position.x -= m_camera.right.x * 10.f * dt;
                m_camera.position.z -= m_camera.right.z * 10.f * dt;
            } break;
            case Camera::Movement::UP: {
                m_camera.position.y += 10.f * dt;
            } break;
            case Camera::Movement::DOWN: m_camera.position.y -= 10.f * dt; break;
        }
    }

    void UpdateCamera(float dt)
    try {
        PROFILE_FUNCTION();
        using Camera = pivot::builtins::Camera;

        if (button.test(static_cast<std::size_t>(Window::Key::Z)))
            processKeyboard(Camera::FORWARD, dt);
        else if (button.test(static_cast<std::size_t>(Window::Key::S)))
            processKeyboard(Camera::BACKWARD, dt);

        if (button.test(static_cast<std::size_t>(Window::Key::Q)))
            processKeyboard(Camera::LEFT, dt);
        else if (button.test(static_cast<std::size_t>(Window::Key::D)))
            processKeyboard(Camera::RIGHT, dt);

        if (button.test(static_cast<std::size_t>(Window::Key::SPACE)))
            processKeyboard(Camera::UP, dt);
        else if (button.test(static_cast<std::size_t>(Window::Key::LEFT_SHIFT)))
            processKeyboard(Camera::DOWN, dt);
    } catch (const std::exception &e) {
        logger.err(pivot::utils::function_name()) << e.what();
    }

    void onTick(float dt) override
    {
        PROFILE_FUNCTION();
        imGuiTheme.setStyle();
        if (!menuBar.render()) {
            imGuiManager.reset();
            return onTick(dt);
        }
        if (menuBar.shouldDisplayColorwindow()) imGuiTheme.setColors();
        editor.create(*this, m_vulkan_application.pipelineStorage);
        m_paused = !editor.getRun();
        sceneEditor.create();
        if (m_paused) {
            sceneEditor.setAspectRatio(m_vulkan_application.getAspectRatio());
            entity.create();
            entity.hasSelected() ? componentEditor.create(entity.getEntitySelected()) : componentEditor.create();
            systemsEditor.create();
            assetBrowser.create();
            if (entity.hasSelected()) { sceneEditor.DisplayGuizmo(entity.getEntitySelected(), m_camera); }
        }
        UpdateCamera(dt);
        this->setRenderArea(vk::Rect2D{
            .offset =
                {
                    .x = static_cast<int32_t>(sceneEditor.offset.x),
                    .y = static_cast<int32_t>(sceneEditor.offset.y),
                },
            .extent =
                {
                    .width = static_cast<uint32_t>(sceneEditor.size.x),
                    .height = static_cast<uint32_t>(sceneEditor.size.y),
                },
        });
    }

    void onFrameStart() override
    {
        PROFILE_FUNCTION();
        imGuiManager.newFrame();
    }

    void onFrameEnd() override
    {
        PROFILE_FUNCTION();
        ImGuiManager::render();
    }

    void onReset() override
    {
        PROFILE_FUNCTION();
        imGuiManager.reset();
    }

public:
    ImGuiManager imGuiManager;
    Editor editor;
    EntityModule entity;
    ComponentEditor componentEditor;
    SystemsEditor systemsEditor;
    AssetBrowser assetBrowser;
    SceneEditor sceneEditor;
    ImGuiTheme imGuiTheme;
    MenuBar menuBar;
    glm::dvec2 last;

    bool bFirstMouse = true;
    std::bitset<UINT16_MAX> button;
};

int main(int argc, const char *argv[])
{
#if !defined(NO_BENCHMARK)

    pivot::benchmark::Instrumentor::get().setThreadName("Editor thread");
    pivot::benchmark::Instrumentor::get().beginSession("Pivot_Startup.json");

#endif

    auto cmdLineArg = getCmdLineArg(argc, argv);
    logger.start(cmdLineArg.verbosity);
    Application app;

    SceneManager::SceneId sceneId;
    if (cmdLineArg.startupScenes.empty()) sceneId = app.loadDefaultScene();
    for (const auto &scenePath: cmdLineArg.startupScenes) sceneId = app.loadScene(scenePath);

    app.changeCurrentScene(sceneId);

    app.init();

#if !defined(NO_BENCHMARK)
    pivot::benchmark::Instrumentor::get().endSession();
#endif

    app.run();

    return 0;
}
