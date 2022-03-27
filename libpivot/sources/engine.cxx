#include <pivot/engine.hxx>

#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Components/RigidBody.hxx>

#include <pivot/components/RenderObject.hxx>
#include <pivot/events/tick.hxx>
#include <pivot/systems/PhysicSystem.hxx>

using namespace pivot;
using namespace pivot::ecs;

namespace pivot
{
Engine::Engine()
{
    m_component_index.registerComponent(Gravity::description);
    m_component_index.registerComponent(RigidBody::description);
    m_component_index.registerComponent(RenderObject::description);
    m_event_index.registerEvent(events::tick);
    m_system_index.registerSystem(systems::physicSystem);
}
}    // namespace pivot
