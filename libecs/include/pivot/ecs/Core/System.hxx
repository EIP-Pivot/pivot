#pragma once

#include "pivot/ecs/Core/types.hxx"
#include <set>

/*! \cond
 */
class System
{
public:
    virtual void Update(float) {};
    virtual void Init() {};
    virtual Signature getSignature() { return Signature(); };
    std::set<Entity> mEntities;
};
/*! \endcond
 */