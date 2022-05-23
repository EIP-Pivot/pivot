#pragma once

#include <pivot/ecs/Core/Systems/description.hxx>
#include <pivot/graphics/AssetStorage.hxx>

namespace pivot::builtins::systems
{
const pivot::ecs::systems::Description makeCollisionSystem(const pivot::graphics::AssetStorage &assetStorage);
}
