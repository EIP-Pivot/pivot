#include <pivot/engine.hxx>

#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Components/RigidBody.hxx>

#include <pivot/builtins/components/RenderObject.hxx>
#include <pivot/builtins/events/tick.hxx>
#include <pivot/builtins/systems/PhysicSystem.hxx>

using namespace pivot;
using namespace pivot::ecs;

namespace pivot
{
Engine::Engine()
{
    m_component_index.registerComponent(Gravity::description);
    m_component_index.registerComponent(RigidBody::description);
    m_component_index.registerComponent(builtins::components::RenderObject::description);
    m_event_index.registerEvent(builtins::events::tick);
    m_system_index.registerSystem(builtins::systems::physicSystem);
}
}    // namespace pivot
