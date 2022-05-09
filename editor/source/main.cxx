#include <chrono>
#include <iostream>
#include <random>

#include <pivot/graphics/VulkanApplication.hxx>
#include <pivot/graphics/Window.hxx>
#include <pivot/graphics/types/RenderObject.hxx>
#include <pivot/graphics/vk_utils.hxx>

#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Components/RigidBody.hxx>
#include <pivot/ecs/Components/Tag.hxx>

#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>

#include <pivot/builtins/systems/ControlSystem.hxx>
#include <pivot/internal/camera.hxx>

#include <pivot/ecs/Core/Systems/description.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>

#include <pivot/ecs/Core/Event/description.hxx>
#include <pivot/ecs/Core/Event/index.hxx>

#include <Logger.hpp>

#include "pivot/graphics/Renderer/CullingRenderer.hxx"
#include "pivot/graphics/Renderer/GraphicsRenderer.hxx"
#include "pivot/graphics/Renderer/ImGuiRenderer.hxx"

#include <pivot/ecs/Core/Scene.hxx>
#include <pivot/ecs/Core/SceneManager.hxx>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ImGuiCore/ComponentEditor.hxx"
#include "ImGuiCore/Editor.hxx"
#include "ImGuiCore/EntityModule.hxx"
#include "ImGuiCore/ImGuiManager.hxx"
#include "ImGuiCore/SystemsEditor.hxx"

#include <pivot/builtins/events/tick.hxx>
#include <pivot/engine.hxx>

using namespace pivot::ecs;
using Window = pivot::graphics::Window;

class Application : public pivot::Engine
{
public:
    Application()
        : Engine(),
          imGuiManager(getSceneManager()),
          editor(getSceneManager(), getCurrentScene()),
          entity(getCurrentScene()),
          systemsEditor(m_system_index, getCurrentScene()),
          componentEditor(m_component_index, getCurrentScene()){};

    SceneManager::SceneId loadDefaultScene() { return registerScene("Default"); }

    void init()
    {
        auto &window = m_vulkan_application.window;

        Scene &scene = *getCurrentScene();

        window.captureCursor(true);
        window.setKeyReleaseCallback(Window::Key::LEFT_ALT, [&](Window &window, const Window::Key key) {
            window.captureCursor(!window.captureCursor());
            bFirstMouse = window.captureCursor();
            button.reset();
        });
        window.setKeyReleaseCallback(Window::Key::V,
                                     [&](Window &window, const Window::Key key) { scene.switchCamera(); });

        auto key_lambda_press = [&](Window &window, const Window::Key key) {
            if (window.captureCursor()) button.set(static_cast<std::size_t>(key));
        };
        auto key_lambda_release = [&](Window &window, const Window::Key key) {
            if (window.captureCursor()) button.reset(static_cast<std::size_t>(key));
        };
        // Press action
        window.setKeyPressCallback(Window::Key::W, key_lambda_press);
        window.setKeyPressCallback(Window::Key::S, key_lambda_press);
        window.setKeyPressCallback(Window::Key::D, key_lambda_press);
        window.setKeyPressCallback(Window::Key::A, key_lambda_press);
        window.setKeyPressCallback(Window::Key::SPACE, key_lambda_press);
        window.setKeyPressCallback(Window::Key::LEFT_SHIFT, key_lambda_press);
        // Release action
        window.setKeyReleaseCallback(Window::Key::W, key_lambda_release);
        window.setKeyReleaseCallback(Window::Key::S, key_lambda_release);
        window.setKeyReleaseCallback(Window::Key::D, key_lambda_release);
        window.setKeyReleaseCallback(Window::Key::A, key_lambda_release);
        window.setKeyReleaseCallback(Window::Key::SPACE, key_lambda_release);
        window.setKeyReleaseCallback(Window::Key::LEFT_SHIFT, key_lambda_release);

        window.setMouseMovementCallback([&](Window &window, const glm::dvec2 pos) {
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

        m_vulkan_application.assetStorage.loadModels("cube.obj",
                                                     "spruce_planks.gltf",
                                                     "light_gray_wool.gltf",
                                                     "white_wool.gltf",
                                                     "jungle_planks.gltf",
                                                     "oak_planks.gltf",
                                                     "iron_block.gltf",
                                                     "glass.gltf",
                                                     "glowstone.gltf",
                                                     "quartz_block.gltf",
                                                     "oak_log.gltf"
                                                     );
        m_vulkan_application.assetStorage.loadTextures("violet.png");
    }
    void processKeyboard(const pivot::builtins::Camera::Movement direction, float dt) noexcept
    {
        using Camera = pivot::builtins::Camera;
        switch (direction) {
            case Camera::Movement::FORWARD: {
                m_camera.position.x += m_camera.front.x * 2.5f * (dt * 50);
                m_camera.position.z += m_camera.front.z * 2.5f * (dt * 50);
            } break;
            case Camera::Movement::BACKWARD: {
                m_camera.position.x -= m_camera.front.x * 2.5f * (dt * 50);
                m_camera.position.z -= m_camera.front.z * 2.5f * (dt * 50);
            } break;
            case Camera::Movement::RIGHT: {
                m_camera.position.x += m_camera.right.x * 2.5f * (dt * 50);
                m_camera.position.z += m_camera.right.z * 2.5f * (dt * 50);
            } break;
            case Camera::Movement::LEFT: {
                m_camera.position.x -= m_camera.right.x * 2.5f * (dt * 50);
                m_camera.position.z -= m_camera.right.z * 2.5f * (dt * 50);
            } break;
            case Camera::Movement::UP: {
                m_camera.position.y += 2.5f * (dt * 20);
            } break;
            case Camera::Movement::DOWN: m_camera.position.y -= 2.5f * (dt * 20); break;
        }
    }

    void UpdateCamera(float dt)
    {
        using Camera = pivot::builtins::Camera;
        try {
            if (button.test(static_cast<std::size_t>(Window::Key::W)))
                processKeyboard(Camera::FORWARD, dt);
            else if (button.test(static_cast<std::size_t>(Window::Key::S)))
                processKeyboard(Camera::BACKWARD, dt);

            if (button.test(static_cast<std::size_t>(Window::Key::A)))
                processKeyboard(Camera::LEFT, dt);
            else if (button.test(static_cast<std::size_t>(Window::Key::D)))
                processKeyboard(Camera::RIGHT, dt);

            if (button.test(static_cast<std::size_t>(Window::Key::SPACE)))
                processKeyboard(Camera::UP, dt);
            else if (button.test(static_cast<std::size_t>(Window::Key::LEFT_SHIFT)))
                processKeyboard(Camera::DOWN, dt);
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void onTick(float dt) override
    {
        imGuiManager.newFrame(*this);

        editor.create(*this, m_vulkan_application.pipelineStorage);
        m_paused = !editor.getRun();
        if (m_paused) {
            editor.setAspectRatio(m_vulkan_application.getAspectRatio());
            entity.create();
            entity.hasSelected() ? componentEditor.create(entity.getEntitySelected()) : componentEditor.create();
            systemsEditor.create();

            if (entity.hasSelected()) { editor.DisplayGuizmo(entity.getEntitySelected(), m_camera); }
        }
        UpdateCamera(dt);
        imGuiManager.render();
    }

public:
    ImGuiManager imGuiManager;
    Editor editor;
    EntityModule entity;
    ComponentEditor componentEditor;
    SystemsEditor systemsEditor;
    glm::dvec2 last;

    bool bFirstMouse = true;
    std::bitset<UINT16_MAX> button;
};

int main(int argc, char *argv[])
{
    logger.start();
    Application app;

    SceneManager::SceneId sceneId;
    if (argc == 2) {
        sceneId = app.loadScene(argv[1]);
    } else {
        sceneId = app.loadDefaultScene();
    }
    app.changeCurrentScene(sceneId);

    app.init();
    app.run();
    return 0;
}
