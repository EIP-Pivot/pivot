#pragma once

#include <pivot/ecs/Core/Systems/description.hxx>

using namespace pivot::ecs;

void controlSystem(const systems::Description &, component::ArrayCombination &, const event::EventWithComponent &);
