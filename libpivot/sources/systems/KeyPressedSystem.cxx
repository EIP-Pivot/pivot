#include "pivot/systems/KeyPressedSystem.hxx"

using namespace pivot::ecs;

void controlSystem(const systems::Description &systemDescription, component::ArrayCombination &entities,
                   const event::EventWithComponent &event)
{
    std::cout << "key pressed" << std::endl;
}
