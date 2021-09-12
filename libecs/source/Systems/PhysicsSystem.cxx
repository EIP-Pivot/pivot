#include "pivot/ecs/Systems/PhysicsSystem.hxx"

#include "pivot/ecs/Components/Gravity.hxx"
#include "pivot/ecs/Components/RigidBody.hxx"
#include "pivot/ecs/Components/Transform.hxx"
#include "pivot/ecs/Core/Coordinator.hxx"

extern Coordinator gCoordinator;

void PhysicsSystem::Init() {}

void PhysicsSystem::Update(float dt)
{
    for (auto const &entity: mEntities) {
        auto &rigidBody = gCoordinator.GetComponent<RigidBody>(entity);
        auto &transform = gCoordinator.GetComponent<Transform>(entity);

        // Forces
        auto const &gravity = gCoordinator.GetComponent<Gravity>(entity);

        transform.position += rigidBody.velocity * dt;

        rigidBody.velocity += gravity.force * dt;
    }
}