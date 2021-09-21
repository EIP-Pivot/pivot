#pragma once

#include "pivot/ecs/Core/System.hxx"
#include "pivot/ecs/Core/Event.hxx"
#include "pivot/graphics/Window.hxx"

class ControlSystem : public System
{
public:
	void Init();

	void Update(float dt);

private:
	std::bitset<UINT16_MAX> button;

	void InputListener(Event& event);
};