#pragma once

#include "pivot/ecs/Core/types.hxx"
#include <set>

/// @class System
///
/// @brief Interface for system implementation
class System
{
public:
    /// Update your system, this function will be called every frame
    virtual void Update(float){};
    /// Init you system and pass the needed variable
    virtual void Init(){};
    /// Get signature for this system, the signature is set with component needed
    virtual Signature getSignature() { return Signature(); };
    /// List of entities that match with signature
    std::set<Entity> mEntities;
};