#pragma once

#include "pivot/ecs/Core/System.hxx"

class PhysicsSystem : public System
{
public:
    void Init();

    virtual void Update(float dt);
};