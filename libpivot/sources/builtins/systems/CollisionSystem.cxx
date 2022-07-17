#include <Logger.hpp>

#include <pivot/graphics/types/RenderObject.hxx>

#include <pivot/builtins/components/Transform.hxx>
#include <pivot/builtins/events/collision.hxx>
#include <pivot/builtins/events/tick.hxx>
#include <pivot/builtins/systems/CollisionSystem.hxx>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/FlagComponentStorage.hxx>

using namespace pivot::ecs;
using namespace pivot::builtins::components;
using namespace pivot::builtins::systems::details;
using AABB = pivot::graphics::gpu_object::AABB;

namespace
{
std::vector<event::Event> collisionSystemImpl(std::reference_wrapper<const pivot::graphics::AssetStorage> assetStorage,
                                              const systems::Description &, component::ArrayCombination &cmb,
                                              const event::EventWithComponent &)
{
    auto collidableStorage = dynamic_cast<const component::FlagComponentStorage &>(cmb.arrays()[0].get());
    auto transformArray =
        dynamic_cast<component::DenseTypedComponentArray<pivot::graphics::Transform> &>(cmb.arrays()[1].get())
            .getData();
    auto renderObjectArray =
        dynamic_cast<component::DenseTypedComponentArray<pivot::graphics::RenderObject> &>(cmb.arrays()[2].get())
            .getData();

    std::vector<EntityAABB> entityAABB;

    for (Entity entity: collidableStorage.getData()) {
        glm::vec3 position = transformArray[entity].position;
        auto &mesh = renderObjectArray[entity].meshID;
        auto bounding_box = assetStorage.get().get_optional<AABB>(mesh);
        if (bounding_box.has_value()) {
            entityAABB.emplace_back(bounding_box->get().low + position, bounding_box->get().high + position, entity);
        }
    }

    auto collisions = getEntityCollisions(entityAABB);
    std::vector<event::Event> collision_events{};

    for (auto [entity1, entity2]: collisions) {
        logger.debug() << "Collision between entity " << entity1 << " and entity " << entity2;
        collision_events.push_back(
            event::Event{pivot::builtins::events::collision, {entity1, entity2}, data::Value{data::Void{}}});
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
                float d2x = box1.low.x - box2.high.x;
                float d2y = box1.low.y - box2.high.y;

                if (d1x > 0.0f || d1y > 0.0f) continue;
                if (d2x > 0.0f || d2y > 0.0f) continue;

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
