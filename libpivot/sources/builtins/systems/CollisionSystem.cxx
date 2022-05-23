#include <Logger.hpp>

#include <pivot/graphics/types/RenderObject.hxx>

#include <pivot/builtins/components/Transform.hxx>
#include <pivot/builtins/events/tick.hxx>
#include <pivot/builtins/systems/CollisionSystem.hxx>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/FlagComponentStorage.hxx>

using namespace pivot::ecs;
using namespace pivot::builtins::components;
using AABB = pivot::graphics::gpu_object::AABB;

namespace
{
struct GlobalAABB {
    glm::vec3 low;
    glm::vec3 high;
    Entity entity;
};

void collisionSystemImpl(const pivot::graphics::AssetStorage &assetStorage,
                         const systems::Description &systemDescription, component::ArrayCombination &cmb,
                         const event::EventWithComponent &event)
{
    auto collidableStorage = dynamic_cast<const component::FlagComponentStorage &>(cmb.arrays()[0].get());
    auto transformArray =
        dynamic_cast<component::DenseTypedComponentArray<pivot::graphics::Transform> &>(cmb.arrays()[1].get())
            .getData();
    auto renderObjectArray =
        dynamic_cast<component::DenseTypedComponentArray<pivot::graphics::RenderObject> &>(cmb.arrays()[2].get())
            .getData();

    std::vector<GlobalAABB> globalAABB;

    for (Entity entity: collidableStorage.getData()) {
        // TODO: Do the collision checks I guess ?
        glm::vec3 position = transformArray[entity].position;
        auto &mesh = renderObjectArray[entity].meshID;
        auto bounding_box = assetStorage.get_optional<AABB>(mesh);
        if (bounding_box.has_value()) {
            globalAABB.emplace_back(bounding_box->get().low + position, bounding_box->get().high + position, entity);
        }
    }

    std::vector<std::pair<Entity, Entity>> collisions;

    for (auto box1: globalAABB) {
        for (auto box2: globalAABB) {
            if (box1.entity == box2.entity) continue;
            float d1x = box2.low.x - box1.high.x;
            float d1y = box2.low.y - box1.high.y;
            float d2x = box1.low.x - box2.high.x;
            float d2y = box1.low.y - box2.high.y;

            if (d1x > 0.0f || d1y > 0.0f) continue;
            if (d2x > 0.0f || d2y > 0.0f) continue;

            collisions.emplace_back(box1.entity, box2.entity);
        }
    }

    for (auto [entity1, entity2]: collisions) {
        logger.debug() << "Collision between entity " << entity1 << " and entity " << entity2;
    }
}
}    // namespace

namespace pivot::builtins::systems
{
const pivot::ecs::systems::Description makeCollisionSystem(const pivot::graphics::AssetStorage &assetStorage)
{
    return {
        .name = "Collision System",
        .systemComponents =
            {
                "Collidable",
                "Transform",
                "RenderObject",
            },
        .eventListener = events::tick,
        .provenance = pivot::ecs::Provenance::builtin(),
        .system = std::bind_front(collisionSystemImpl, assetStorage),
    };
}
}    // namespace pivot::builtins::systems
