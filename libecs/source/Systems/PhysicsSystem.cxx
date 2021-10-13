#include "pivot/ecs/Systems/PhysicsSystem.hxx"

#include "pivot/ecs/Components/Gravity.hxx"
#include "pivot/ecs/Components/RigidBody.hxx"
#include "pivot/ecs/Components/Transform.hxx"
#include "pivot/ecs/Core/SceneManager.hxx"

extern SceneManager gSceneManager;

void PhysicsSystem::Init() {}

void PhysicsSystem::Update(float dt)
{
    for (auto const &entity: mEntities) {
        auto &rigidBody = gSceneManager.getCurrentLevel().GetComponent<RigidBody>(entity);
        auto &transform = gSceneManager.getCurrentLevel().GetComponent<pivot::ecs::component::Transform>(entity);

        // Forces
        auto const &gravity = gSceneManager.getCurrentLevel().GetComponent<Gravity>(entity);

        transform.position += rigidBody.velocity * dt;

        rigidBody.velocity += gravity.force * dt;
    }
}