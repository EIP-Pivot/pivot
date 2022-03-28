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

#include <pivot/ecs/Core/Event.hxx>
#include <pivot/ecs/ecs.hxx>

#include <pivot/internal/camera.hxx>
#include <pivot/systems/ControlSystem.hxx>

#include <pivot/ecs/Core/Systems/description.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>

#include <pivot/ecs/Core/Event/description.hxx>
#include <pivot/ecs/Core/Event/index.hxx>

#include <Logger.hpp>

// #include "Scene.hxx"
#include "Systems/PhysicsSystem.hxx"
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

#include "FrameLimiter.hpp"

class Application : public pivot::graphics::VulkanApplication
{
public:
    static const event::Description tick;

    Application()
        : VulkanApplication(),
          editor(Editor()),
          camera(editor.getCamera()),
          systemsEditor(systemIndex),
          componentEditor(componentIndex){};

    void loadScene()
    {
        LevelId defaultScene = editor.addScene("Default");
        gSceneManager.setCurrentLevelId(defaultScene);
    }

    void init()
    {
        eventIndex.registerEvent(tick);

        pivot::ecs::systems::Description description{
            .name = "Physics System",
            .systemComponents =
                {
                    "Gravity",
                    "RigidBody",
                },
            .eventListener = tick,
            .system = &physicsSystem,
        };
        systemIndex.registerSystem(description);

        loadScene();

        window.captureCursor(true);
        window.setKeyReleaseCallback(Window::Key::LEFT_ALT, [&](Window &window, const Window::Key key) {
            window.captureCursor(!window.captureCursor());
            bFirstMouse = window.captureCursor();
            button.reset();
        });
        window.setKeyReleaseCallback(Window::Key::V, [&](Window &window, const Window::Key key) {
            gSceneManager.getCurrentLevel().switchCamera();
        });

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
#ifdef CULLING_DEBUG
        window.setKeyPressCallback(Window::Key::C, [this](Window &window, const Window::Key key) {
            if (window.captureCursor())
                this->editor.cullingCameraFollowsCamera = !this->editor.cullingCameraFollowsCamera;
        });
#endif
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
            ControlSystem::processMouseMovement(camera, glm::dvec2(xoffset, yoffset));
        });
        assetStorage.loadModels("../editor/assets/cube.obj");
        assetStorage.loadTextures("../editor/assets/violet.png");
    }
    void processKeyboard(const Camera::Movement direction, float dt) noexcept
    {
        switch (direction) {
            case Camera::Movement::FORWARD: {
                camera.position.x += camera.front.x * 2.5f * (dt * 500);
                camera.position.z += camera.front.z * 2.5f * (dt * 500);
            } break;
            case Camera::Movement::BACKWARD: {
                camera.position.x -= camera.front.x * 2.5f * (dt * 500);
                camera.position.z -= camera.front.z * 2.5f * (dt * 500);
            } break;
            case Camera::Movement::RIGHT: {
                camera.position.x += camera.right.x * 2.5f * (dt * 500);
                camera.position.z += camera.right.z * 2.5f * (dt * 500);
            } break;
            case Camera::Movement::LEFT: {
                camera.position.x -= camera.right.x * 2.5f * (dt * 500);
                camera.position.z -= camera.right.z * 2.5f * (dt * 500);
            } break;
            case Camera::Movement::UP: {
                camera.position.y += 2.5f * (dt * 500);
            } break;
            case Camera::Movement::DOWN: camera.position.y -= 2.5f * (dt * 500); break;
        }
    }

    void UpdateCamera(float dt)
    {
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

    void run()
    {
        float dt = 0.0f;
        float fov = 27.f;
        bool useWindow = true;
        Entity currentEdit = 0;
        gridSize = 100.f;
        this->VulkanApplication::init();
        FrameLimiter<60> fpsLimiter;
        while (!window.shouldClose()) {
            auto startTime = std::chrono::high_resolution_clock::now();
            window.pollEvent();

            imGuiManager.newFrame();

            editor.create();
            if (!editor.getRun()) {
                editor.setAspectRatio(getAspectRatio());
                entity.create();
                entity.hasSelected() ? componentEditor.create(entity.getEntitySelected()) : componentEditor.create();
                systemsEditor.create();

                // if (entity.hasSelected() &&
                //     gSceneManager.getCurrentLevel().hasComponent<RenderObject>(entity.getEntitySelected())) {
                //     editor.DisplayGuizmo(entity.getEntitySelected());
                // }
            } else {
                gSceneManager.getCurrentLevel().getEventManager().sendEvent({tick, {}, data::Value(dt)});
            }
            UpdateCamera(dt);

            imGuiManager.render();

            auto aspectRatio = getAspectRatio();
            float fov = 80;

            auto &cm = gSceneManager.getCurrentLevel().getComponentManager();
            auto renderobject_id = cm.GetComponentId("RenderObject").value();
            auto &array = cm.GetComponentArray(renderobject_id);
            pivot::ecs::component::DenseTypedComponentArray<RenderObject> &dense_array =
                dynamic_cast<pivot::ecs::component::DenseTypedComponentArray<RenderObject> &>(array);

            auto data = dense_array.getData();
            std::vector<std::reference_wrapper<const pivot::graphics::RenderObject>> objects;
            for (const auto &ro: data) { objects.push_back(ro); }

#ifdef CULLING_DEBUG
            if (editor.cullingCameraFollowsCamera) editor.cullingCamera = camera;
#endif
            draw(objects, pivot::internals::getGPUCameraData(camera, fov, aspectRatio)
#ifdef CULLING_DEBUG
                              ,
                 std::make_optional(pivot::internals::getGPUCameraData(editor.cullingCamera, fov, aspectRatio))
#endif
            );
            fpsLimiter.sleep();
            auto stopTime = std::chrono::high_resolution_clock::now();
            dt = std::chrono::duration<float>(stopTime - startTime).count();
        }
    }

public:
    ImGuiManager imGuiManager;
    Editor editor;
    EntityModule entity;
    ComponentEditor componentEditor;
    SystemsEditor systemsEditor;
    Camera &camera;
    glm::dvec2 last;

    bool bFirstMouse = true;
    std::bitset<UINT16_MAX> button;
    int gridSize;

    component::Index componentIndex;
    systems::Index systemIndex;
    event::Index eventIndex;
};

const event::Description Application::tick = {
    .name = "Tick",
    .entities = {},
    .payload = pivot::ecs::data::BasicType::Number,
};

int main()
try {
    logger.start();
    Application app;
    app.init();
    app.run();
    return 0;
} catch (std::exception &e) {
    logger.err("THROW") << e.what();
    return 1;
}
