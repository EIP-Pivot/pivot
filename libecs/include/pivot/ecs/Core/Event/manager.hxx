#pragma once

#include "pivot/ecs/Core/Event/description.hxx"
#include "pivot/ecs/Core/Data/value.hxx"
#include "pivot/ecs/Core/types.hxx"

#include <vector>
#include <any>

namespace pivot::ecs::event
{

class Manager
{
public:
    
    void registerEvent(const Description &description);

    void sendEvent(const std::string &eventName, const data::Value &value, const std::vector<Entity> &entities);

private:
};


}