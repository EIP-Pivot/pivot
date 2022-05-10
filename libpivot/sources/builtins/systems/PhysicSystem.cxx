#include <pivot/builtins/events/tick.hxx>
#include <pivot/builtins/systems/PhysicSystem.hxx>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>

#include <pivot/builtins/components/Transform.hxx>
#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Components/RigidBody.hxx>

using namespace pivot::ecs;
using namespace pivot::builtins::components;

namespace
{
void physicsSystemImpl(const systems::Description &systemDescription, component::ArrayCombination &cmb,
                       const event::EventWithComponent &event)
{
    auto dt = (float)std::get<double>(event.event.payload);

    auto gravityArray = dynamic_cast<component::DenseTypedComponentArray<Gravity> &>(cmb.arrays()[0].get()).getData();
    auto rigidBodyArray =
        dynamic_cast<component::DenseTypedComponentArray<RigidBody> &>(cmb.arrays()[1].get()).getData();
    auto transformArray =
        dynamic_cast<component::DenseTypedComponentArray<Transform> &>(cmb.arrays()[2].get()).getData();

    auto maxEntity = std::min({gravityArray.size(), rigidBodyArray.size(), transformArray.size()});
    for (std::size_t entity = 0; entity <= maxEntity; entity++) {
        auto &gravity = gravityArray[entity];
        auto &rigidBody = rigidBodyArray[entity];
        auto &transform = transformArray[entity];

        if (gravity.force != glm::vec3(0)) { rigidBody.acceleration = gravity.force; }
        rigidBody.velocity += rigidBody.acceleration * dt;
        transform.position += rigidBody.velocity * dt;
    }
}
}    // namespace

namespace pivot::builtins::systems
{
const pivot::ecs::systems::Description physicSystem{
    .name = "Physics System",
    .systemComponents =
        {
            "Gravity",
            "RigidBody",
            "Transform",
        },
    .eventListener = events::tick,
    .system = &physicsSystemImpl,
};
}
