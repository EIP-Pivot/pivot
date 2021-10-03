#include "pivot/ecs/Systems/ControlSystem.hxx"

#include "pivot/ecs/Components/Transform.hxx"
#include "pivot/ecs/Core/Coordinator.hxx"

extern Coordinator gCoordinator;

void ControlSystem::Init()
{
    gCoordinator.AddEventListener(Events::Window::INPUT,
                                  std::bind(&ControlSystem::InputListener, this, std::placeholders::_1));
    gCoordinator.AddEventListener(Events::Window::MOUSE,
                                  std::bind(&ControlSystem::MouseListener, this, std::placeholders::_1));
}

void ControlSystem::Update(float dt)
{
    try {
        for (auto &entity: mEntities) {
            auto &transform = gCoordinator.GetComponent<Transform>(entity);
            auto &camera = gCoordinator.GetComponent<Camera>(entity);

            if (button.test(static_cast<std::size_t>(Window::Key::W))) {
                processKeyboard(camera, Camera::FORWARD);
                // transform.position.z += (dt * 10.0f);
            } else if (button.test(static_cast<std::size_t>(Window::Key::S))) {
                processKeyboard(camera, Camera::BACKWARD);
                // transform.position.z -= (dt * 10.0f);
            }

            if (button.test(static_cast<std::size_t>(Window::Key::A))) {
                processKeyboard(camera, Camera::LEFT);
                // transform.position.x += (dt * 10.0f);
            } else if (button.test(static_cast<std::size_t>(Window::Key::D))) {
                processKeyboard(camera, Camera::RIGHT);
                // transform.position.x -= (dt * 10.0f);
            }

            if (button.test(static_cast<std::size_t>(Window::Key::SPACE))) {
                processKeyboard(camera, Camera::UP);
                // transform.position.x += (dt * 10.0f);
            } else if (button.test(static_cast<std::size_t>(Window::Key::LEFT_SHIFT))) {
                processKeyboard(camera, Camera::DOWN);
                // transform.position.x -= (dt * 10.0f);
            }
            if (offset) {
                ControlSystem::processMouseMovement(camera, *offset);
                offset = std::nullopt;
            }
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

void ControlSystem::InputListener(Event &event)
{
    button = event.GetParam<std::bitset<UINT16_MAX>>(Events::Window::Input::INPUT);
}

void ControlSystem::MouseListener(Event &event) { offset = event.GetParam<glm::dvec2>(Events::Window::Mouse::MOUSE); }

void ControlSystem::processKeyboard(Camera &cam, const Camera::Movement direction) noexcept
{
    switch (direction) {
        case Camera::Movement::FORWARD: {
            cam.position.x += cam.front.x * SPEED;
            cam.position.z += cam.front.z * SPEED;
        } break;
        case Camera::Movement::BACKWARD: {
            cam.position.x -= cam.front.x * SPEED;
            cam.position.z -= cam.front.z * SPEED;
        } break;
        case Camera::Movement::RIGHT: {
            cam.position.x += cam.right.x * SPEED;
            cam.position.z += cam.right.z * SPEED;
        } break;
        case Camera::Movement::LEFT: {
            cam.position.x -= cam.right.x * SPEED;
            cam.position.z -= cam.right.z * SPEED;
        } break;
        case Camera::Movement::UP: {
            cam.position.y += JUMP;
        } break;
        case Camera::Movement::DOWN: cam.position.y -= SPEED; break;
    }
}

void ControlSystem::processMouseMovement(Camera &cam, const glm::dvec2 &offset)
{
    cam.yaw += offset.x * SENSITIVITY;
    cam.pitch += offset.y * SENSITIVITY;

    if (CONSTRAIN_PITCH) {
        if (cam.pitch > 89.0f) cam.pitch = 89.0f;
        if (cam.pitch < -89.0f) cam.pitch = -89.0f;
    }
    cam.updateCameraVectors();
}
