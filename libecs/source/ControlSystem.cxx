#include "pivot/ecs/Systems/ControlSystem.hxx"

#include "pivot/ecs/Components/Gravity.hxx"
#include "pivot/ecs/Components/RigidBody.hxx"
#include "pivot/ecs/Components/Transform.hxx"
#include "pivot/ecs/Core/Coordinator.hxx"

extern Coordinator gCoordinator;

void ControlSystem::Init()
{
	gCoordinator.AddEventListener(Events::Window::INPUT, std::bind(&ControlSystem::InputListener, this, std::placeholders::_1));
}

void ControlSystem::Update(float dt)
{
    for (auto& entity : mEntities)
	{
		auto& transform = gCoordinator.GetComponent<Transform>(entity);


		if (mButtons == InputButtons::W)
		{
            std::cout << "W" << std::endl;
			// transform.position.z += (dt * 10.0f);
		}
		else if (mButtons == InputButtons::S)
		{
            std::cout << "S" << std::endl;
			// transform.position.z -= (dt * 10.0f);
		}


		if (mButtons == InputButtons::A)
		{
            std::cout << "A" << std::endl;
			// transform.position.x += (dt * 10.0f);
		}
		else if (mButtons == InputButtons::D)
		{
            std::cout << "D" << std::endl;
			// transform.position.x -= (dt * 10.0f);
		}
	}
}

void ControlSystem::InputListener(Event& event)
{
	mButtons = event.GetParam<InputButtons>(Events::Window::Input::INPUT);
}