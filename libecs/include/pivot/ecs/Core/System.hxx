#pragma once

#include "pivot/ecs/Core/types.hxx"
#include <set>

/*! \cond
 */
class System
{
public:
    std::set<Entity> mEntities;
};
/*! \endcond
 */