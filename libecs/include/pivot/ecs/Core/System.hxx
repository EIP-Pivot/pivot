#pragma once

#include "pivot/ecs/Core/types.hxx"
#include <set>

class System
{
public:
    std::set<Entity> mEntities;
};