
#include <pivot/graphics/Window.hxx>

#include <pivot/builtins/systems/ControlSystem.hxx>

#include <pivot/ecs/Core/Event/description.hxx>

#include <pivot/ecs/Core/Scene.hxx>
#include <pivot/ecs/Core/SceneManager.hxx>

#include <glm/gtx/quaternion.hpp>

#include "CmdLineArg.hxx"
#include "ImGuiCore/ImGuiTheme.hxx"
#include "ImGuiCore/MenuBar.hxx"
#include "WindowsManager.hxx"

#include <pivot/engine.hxx>
#include <pivot/utility/benchmark.hxx>

using namespace pivot::ecs;
using namespace pivot::editor;
using Window = pivot::graphics::Window;

class Application : public pivot::Engine
{
public:
    Application()
        : Engine(),
          menuBar(getSceneManager(), *this),
          windowsManager(m_component_index, m_system_index, getSceneManager(), getCurrentScene(),
                         m_vulkan_application.assetStorage, m_vulkan_application.pipelineStorage, *this, m_paused)
    {
        for (const auto &directoryEntry: std::filesystem::recursive_directory_iterator(m_asset_directory / "Editor")) {
            if (directoryEntry.is_directory()) continue;
            loadAsset(directoryEntry.path(), false);
        }
    }

    SceneManager::SceneId loadDefaultScene() { return registerScene("Default"); }

    void init()
    {
        PROFILE_FUNCTION();
        m_window.addKeyReleaseCallback(Window::Key::LEFT_ALT,
                                       [&](Window &window, const Window::Key, const Window::Modifier) {
                                           window.captureCursor(!window.captureCursor());
                                           bFirstMouse = window.captureCursor();
                                           button.reset();
                                       });

        auto key_lambda_press = [&](Window &window, const Window::Key key, const Window::Modifier) {
            if (window.captureCursor()) button.set(static_cast<std::size_t>(key));
        };
        auto key_lambda_release = [&](Window &window, const Window::Key key, const Window::Modifier) {
            if (window.captureCursor()) button.reset(static_cast<std::size_t>(key));
        };
        // Press action
        m_window.addKeyPressCallback(Window::Key::Z, key_lambda_press);
        m_window.addKeyPressCallback(Window::Key::Q, key_lambda_press);
        m_window.addKeyPressCallback(Window::Key::S, key_lambda_press);
        m_window.addKeyPressCallback(Window::Key::D, key_lambda_press);
        m_window.addKeyPressCallback(Window::Key::SPACE, key_lambda_press);
        m_window.addKeyPressCallback(Window::Key::LEFT_SHIFT, key_lambda_press);
        // Release action
        m_window.addKeyReleaseCallback(Window::Key::Z, key_lambda_release);
        m_window.addKeyReleaseCallback(Window::Key::Q, key_lambda_release);
        m_window.addKeyReleaseCallback(Window::Key::S, key_lambda_release);
        m_window.addKeyReleaseCallback(Window::Key::D, key_lambda_release);
        m_window.addKeyReleaseCallback(Window::Key::SPACE, key_lambda_release);
        m_window.addKeyReleaseCallback(Window::Key::LEFT_SHIFT, key_lambda_release);

        m_window.addMouseMovementCallback([&](Window &window, const glm::dvec2 pos) {
            if (!window.captureCursor()) return;

            if (bFirstMouse) {
                last = pos;
                bFirstMouse = false;
            }
            auto xoffset = pos.x - last.x;
            auto yoffset = last.y - pos.y;

            last = pos;
            pivot::builtins::systems::ControlSystem::processMouseMovement(getCurrentCamera().camera,
                                                                          glm::dvec2(xoffset, yoffset));
        });
        m_window.addKeyPressCallback(Window::Key::ESCAPE,
                                     [&](Window &window, const Window::Key, const Window::Modifier) {
                                         logger.debug() << "Escape";
                                         if (this->m_paused) {
                                             window.shouldClose(true);
                                         } else {
                                             m_paused = false;
                                         }
                                     });
        m_vulkan_application.buildAssetStorage(pivot::graphics::AssetStorage::BuildFlagBits::eReloadOldAssets);
        // resize or loading asset reset imgui -> style reset
        //        ImGuiTheme::setStyle();
    }

    void processKeyboard(pivot::internals::LocationCamera camera, pivot::internals::LocationCamera::Movement direction,
                         float dt) noexcept
    {
        PROFILE_FUNCTION();
        float speed = 30.0f;
        using Movement = pivot::internals::LocationCamera::Movement;
        glm::vec3 &camera_position = camera.transform.position;
        pivot::internals::LocationCamera::Directions camera_directions = camera.getDirections();
        switch (direction) {
            case Movement::FORWARD: {
                camera_position.x += camera_directions.front.x * speed * dt;
                camera_position.z += camera_directions.front.z * speed * dt;
            } break;
            case Movement::BACKWARD: {
                camera_position.x -= camera_directions.front.x * speed * dt;
                camera_position.z -= camera_directions.front.z * speed * dt;
            } break;
            case Movement::RIGHT: {
                camera_position.x += camera_directions.right.x * speed * dt;
                camera_position.z += camera_directions.right.z * speed * dt;
            } break;
            case Movement::LEFT: {
                camera_position.x -= camera_directions.right.x * speed * dt;
                camera_position.z -= camera_directions.right.z * speed * dt;
            } break;
            case Movement::UP: {
                camera_position.y += speed * dt;
            } break;
            case Movement::DOWN: camera_position.y -= speed * dt; break;
        }
    }

    void UpdateCamera(pivot::internals::LocationCamera camera, float dt)
    {
        PROFILE_FUNCTION();
        using LocationCamera = pivot::internals::LocationCamera;
        try {
            if (button.test(static_cast<std::size_t>(Window::Key::Z)))
                processKeyboard(camera, LocationCamera::FORWARD, dt);
            else if (button.test(static_cast<std::size_t>(Window::Key::S)))
                processKeyboard(camera, LocationCamera::BACKWARD, dt);

            if (button.test(static_cast<std::size_t>(Window::Key::Q)))
                processKeyboard(camera, LocationCamera::LEFT, dt);
            else if (button.test(static_cast<std::size_t>(Window::Key::D)))
                processKeyboard(camera, LocationCamera::RIGHT, dt);

            if (button.test(static_cast<std::size_t>(Window::Key::SPACE)))
                processKeyboard(camera, LocationCamera::UP, dt);
            else if (button.test(static_cast<std::size_t>(Window::Key::LEFT_SHIFT)))
                processKeyboard(camera, LocationCamera::DOWN, dt);
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void onTick(float dt) override
    {
        PROFILE_FUNCTION();
        imGuiTheme.setStyle();
        if (!menuBar.render()) {
            windowsManager.reset();
            return onTick(dt);
        }
        windowsManager.render();
        if (menuBar.shouldDisplayColorwindow()) imGuiTheme.setColors();
        windowsManager.setAspectRatio(m_vulkan_application.getAspectRatio());
        UpdateCamera(getCurrentCamera(), dt);
        this->setRenderArea(vk::Rect2D{
            .offset =
                {
                    .x = static_cast<int32_t>(windowsManager.workspace.offset.x),
                    .y = static_cast<int32_t>(windowsManager.workspace.offset.y),
                },
            .extent =
                {
                    .width = static_cast<uint32_t>(windowsManager.workspace.size.x),
                    .height = static_cast<uint32_t>(windowsManager.workspace.size.y),
                },
        });
    }

    void onFrameStart() override
    {
#if !defined(NO_BENCHMARK)
        static bool shouldCaptureFrame = menuBar.shouldCaptureFrame();
        if (shouldCaptureFrame) {
            shouldCaptureFrame = false;
            if (pivot::benchmark::Instrumentor::get().isSessionStarted()) {
                pivot::benchmark::Instrumentor::get().endSession();
            } else {
                pivot::benchmark::Instrumentor::get().beginSession("Pivot_Frame.json");
            }
        }
#endif
        PROFILE_FUNCTION();
        windowsManager.newFrame();
    }

    void onFrameEnd() override
    {
        PROFILE_FUNCTION();
        windowsManager.endFrame();
    }

    void onReset() override
    {
        PROFILE_FUNCTION();
        windowsManager.reset();
    }

public:
    ImGuiTheme imGuiTheme;
    MenuBar menuBar;
    WindowsManager windowsManager;
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
