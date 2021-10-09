#pragma once

#include "pivot/ecs/Core/types.hxx"
#include <set>

/*! \cond
 */
class System
{
public:
    virtual void Update(float) {};
    std::set<Entity> mEntities;
};
/*! \endcond
 */