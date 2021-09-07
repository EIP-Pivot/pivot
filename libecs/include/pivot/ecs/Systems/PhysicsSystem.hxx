#pragma once

#include "pivot/ecs/Core/System.hxx"

class PhysicsSystem : public System
{
public:
    void Init();

    void Update(float dt);
};