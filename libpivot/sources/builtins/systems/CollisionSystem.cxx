#include <csignal>

#include <pivot/graphics/types/RenderObject.hxx>

#include <pivot/builtins/components/Transform.hxx>
#include <pivot/builtins/events/collision.hxx>
#include <pivot/builtins/events/tick.hxx>
#include <pivot/builtins/systems/CollisionSystem.hxx>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/FlagComponentStorage.hxx>
#include <pivot/ecs/Core/Component/SynchronizedComponentArray.hxx>
#include <pivot/internal/TransformArray.hxx>

using namespace pivot::ecs;
using namespace pivot::builtins::components;
using namespace pivot::builtins::systems::details;
using AABB = pivot::graphics::gpu_object::AABB;
using Prefab = pivot::graphics::asset::Prefab;

namespace
{
std::vector<event::Event> collisionSystemImpl(std::reference_wrapper<const pivot::graphics::AssetStorage> assetStorage,
                                              const systems::Description &, component::ArrayCombination &cmb,
                                              const event::EventWithComponent &)
{
    logger.trace() << "Collision system run";
    auto collidableStorage = dynamic_cast<const component::FlagComponentStorage &>(cmb.arrays()[0].get());
    const auto &transformArray = dynamic_cast<pivot::internal::SynchronizedTransformArray &>(cmb.arrays()[1].get());
    const auto &renderObjectArray =
        dynamic_cast<component::SynchronizedTypedComponentArray<pivot::graphics::RenderObject> &>(
            cmb.arrays()[2].get());
    std::scoped_lock array_lock(transformArray.getMutex(), renderObjectArray.getMutex());

    auto transformData = transformArray.getData();
    auto renderObjectData = renderObjectArray.getData();

    std::vector<EntityAABB> entityAABB;

    for (Entity entity: collidableStorage.getData()) {
        logger.trace() << "Retrieving AABB for entity " << entity;
        glm::vec3 position = transformData[entity].position;
        auto &prefab_name = renderObjectData[entity].meshID;
        auto prefab = assetStorage.get().get_optional<Prefab>(prefab_name);
        if (!prefab.has_value()) continue;
        auto bounding_box = assetStorage.get().get_optional<AABB>(prefab.value().get().modelIds.at(0));
        if (bounding_box.has_value()) {
            entityAABB.emplace_back(bounding_box->get().low + position, bounding_box->get().high + position, entity);
        }
    }

    logger.trace() << "Collisions between " << entityAABB.size() << " entities with AABB";

    auto collisions = getEntityCollisions(entityAABB);
    std::vector<event::Event> collision_events{};

    for (auto [entity1, entity2]: collisions) {
        logger.debug() << "Collision between entity " << entity1 << " and entity " << entity2;
        collision_events.push_back(
            event::Event{pivot::builtins::events::collision, {entity1, entity2}, data::Value{data::Void{}}});
        collision_events.push_back(
            event::Event{pivot::builtins::events::collision, {entity2, entity1}, data::Value{data::Void{}}});
    }

    return collision_events;
}
}    // namespace

namespace pivot::builtins::systems
{
namespace details
{

    std::vector<std::pair<Entity, Entity>> getEntityCollisions(std::span<const EntityAABB> entityAABB)
    {
        std::vector<std::pair<Entity, Entity>> collisions;

        for (std::size_t i = 0; i < entityAABB.size(); i++) {
            auto &box1 = entityAABB[i];
            for (auto &box2: entityAABB.subspan(i)) {
                if (box1.entity == box2.entity) continue;
                float d1x = box2.low.x - box1.high.x;
                float d1y = box2.low.y - box1.high.y;
                float d1z = box2.low.z - box1.high.z;
                float d2x = box1.low.x - box2.high.x;
                float d2y = box1.low.y - box2.high.y;
                float d2z = box1.low.z - box2.high.z;

                if (d1x > 0.0f || d1y > 0.0f || d1z > 0.0f) continue;
                if (d2x > 0.0f || d2y > 0.0f || d2z > 0.0f) continue;

                collisions.emplace_back(box1.entity, box2.entity);
            }
        }
        return collisions;
    }

}    // namespace details

const pivot::ecs::systems::Description makeCollisionSystem(const pivot::graphics::AssetStorage &assetStorage)
{
    return pivot::ecs::systems::Description{
        .name = "Collision System",
        .entityName = "",
        .systemComponents =
            {
                "Collidable",
                "Transform",
                "RenderObject",
            },
        .eventListener = events::tick,
        .eventComponents = {},
        .provenance = pivot::ecs::Provenance::builtin(),
        .system = std::bind_front(collisionSystemImpl, std::cref(assetStorage)),
    };
}
}    // namespace pivot::builtins::systems
