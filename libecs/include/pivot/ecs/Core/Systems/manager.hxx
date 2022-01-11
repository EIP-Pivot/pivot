#pragma once

#include "pivot/ecs/Core/Component/manager.hxx"
#include "pivot/ecs/Core/EcsException.hxx"
#include "pivot/ecs/Core/EntityManager.hxx"
#include "pivot/ecs/Core/Systems/description.hxx"
#include "pivot/ecs/Core/Systems/function_traits.hxx"
#include "pivot/ecs/Core/Systems/index.hxx"

#include <tuple>
#include <functional>
#include <unordered_map>

namespace pivot::ecs::systems
{

class Manager
{
public:
    bool useSystem(const Description &description);

    void execute(component::Manager &componentManager, EntityManager &entityManager);

private:
    std::unordered_map<std::string, std::function<void(component::Manager &, EntityManager &)>> m_systems;
};

}    // namespace pivot::ecs::systems