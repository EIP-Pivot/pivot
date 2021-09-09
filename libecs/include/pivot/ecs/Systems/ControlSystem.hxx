#pragma once

#include "pivot/ecs/Core/System.hxx"
#include "pivot/ecs/Core/Event.hxx"

class ControlSystem : public System
{
public:
	void Init();

	void Update(float dt);

private:
	std::bitset<8> button;

	void InputListener(Event& event);
};