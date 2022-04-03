#include "Systems/PhysicsSystem.hxx"

using namespace pivot::ecs;

void physicsSystem(const systems::Description &systemDescription, component::ArrayCombination &entities,
                   const event::EventWithComponent &event)
{
    auto dt = (float)std::get<double>(event.event.payload);
    for (auto combination: entities) {
        auto gravity = combination[0].get();
        auto rigidBody = combination[1].get();
        auto renderObject = combination[2].get();

        auto &position = std::get<glm::vec3>(std::get<data::Record>(std::get<data::Record>(renderObject).at("transform")).at("position"));
        auto &velocity = std::get<glm::vec3>(std::get<data::Record>(rigidBody).at("velocity"));
        auto &force = std::get<glm::vec3>(std::get<data::Record>(gravity).at("force"));

        position += velocity * dt;
        velocity += force * dt;

        combination[1].set(rigidBody);
        combination[2].set(renderObject);
    }
}
