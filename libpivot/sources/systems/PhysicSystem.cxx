#include <pivot/events/tick.hxx>
#include <pivot/systems/PhysicSystem.hxx>

using namespace pivot::ecs;
namespace
{
void physicsSystemImpl(const systems::Description &systemDescription, component::ArrayCombination &entities,
                       const event::EventWithComponent &event)
{
    auto dt = (float)std::get<double>(event.event.payload);
    for (auto combination: entities) {
        auto gravity = combination[0].get();
        auto rigidBody = combination[1].get();

        auto &force = std::get<glm::vec3>(std::get<data::Record>(gravity).at("force"));
        force.x += 1;

        combination[0].set(gravity);

        auto &velocity = std::get<glm::vec3>(std::get<data::Record>(rigidBody).at("velocity"));
        velocity += force * dt;
        combination[1].set(rigidBody);
    }
}
}    // namespace

namespace pivot::systems
{
const pivot::ecs::systems::Description physicSystem{
    .name = "Physics System",
    .systemComponents =
        {
            "Gravity",
            "RigidBody",
        },
    .eventListener = events::tick,
    .system = &physicsSystemImpl,
};

}
