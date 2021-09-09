#pragma once

#include "pivot/ecs/Core/System.hxx"
#include "pivot/ecs/Core/Event.hxx"


class Event;


class ControlSystem : public System
{
public:
	void Init();

	void Update(float dt);

private:
	InputButtons mButtons;

	void InputListener(Event& event);
};