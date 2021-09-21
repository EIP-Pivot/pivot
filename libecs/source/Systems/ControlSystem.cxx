#include "pivot/ecs/Systems/ControlSystem.hxx"

#include "pivot/graphics/Camera.hxx"
#include "pivot/ecs/Components/Transform.hxx"
#include "pivot/ecs/Core/Coordinator.hxx"

extern Coordinator gCoordinator;

void ControlSystem::Init()
{
	gCoordinator.AddEventListener(Events::Window::INPUT, std::bind(&ControlSystem::InputListener, this, std::placeholders::_1));
}

void ControlSystem::Update(float dt)
{
    try
    {
        for (auto& entity : mEntities)
        {
            auto& transform = gCoordinator.GetComponent<Transform>(entity);
            auto& camera = gCoordinator.GetComponent<Camera>(entity);

            if (button.test(static_cast<std::size_t>(Window::Key::W)))
            {
                camera.processKeyboard(Camera::FORWARD);
                // transform.position.z += (dt * 10.0f);
            }
            else if (button.test(static_cast<std::size_t>(Window::Key::S)))
            {
                camera.processKeyboard(Camera::BACKWARD);
                // transform.position.z -= (dt * 10.0f);
            }


            if (button.test(static_cast<std::size_t>(Window::Key::A)))
            {
                camera.processKeyboard(Camera::LEFT);
                // transform.position.x += (dt * 10.0f);
            }
            else if (button.test(static_cast<std::size_t>(Window::Key::D)))
            {
                camera.processKeyboard(Camera::RIGHT);
                // transform.position.x -= (dt * 10.0f);
            }

            if (button.test(static_cast<std::size_t>(Window::Key::SPACE)))
            {
                camera.processKeyboard(Camera::UP);
                // transform.position.x += (dt * 10.0f);
            }
            else if (button.test(static_cast<std::size_t>(Window::Key::LEFT_SHIFT)))
            {
                camera.processKeyboard(Camera::DOWN);
                // transform.position.x -= (dt * 10.0f);
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    
}

void ControlSystem::InputListener(Event& event)
{
    button = event.GetParam<std::bitset<UINT16_MAX>>(Events::Window::Input::INPUT);
}