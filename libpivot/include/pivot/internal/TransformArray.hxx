#pragma once

#include <set>
#include <stdexcept>

#include <pivot/builtins/components/Transform.hxx>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/array.hxx>

namespace pivot::internal
{

/** \brief Storage for Transform components
 *
 * This IComponentArray is a DenseTypedComponentArray, but it additionally
 * checks the validity of transform roots:
 *
 * - Roots always point to an entity with a Transform
 *
 * - Roots cannot go past a depth of one
 *
 * - Removing the Transform component of the root removes the root without moving the entity
 */
class TransformArray : public ecs::component::DenseTypedComponentArray<pivot::graphics::Transform>
{
public:
    /// Creates a TransformArray using the Description of Transform.
    TransformArray(ecs::component::Description d): DenseTypedComponentArray<pivot::graphics::Transform>(d) {}

    void setValueForEntity(Entity entity, std::optional<ecs::data::Value> value) override;

    /// Error thrown when setting the root of the transform if the root depth goes over 1
    class RootDepthExceeded : public std::logic_error
    {
    public:
        /// Creates a RootDepthExceeded from the entity on which setValueForEntity was called
        RootDepthExceeded(Entity entity): std::logic_error("Root depth exceeded on entity " + std::to_string(entity)) {}
    };

private:
    // Mapping between every entity and the entities using the entity as a root
    std::vector<std::set<Entity>> m_reverse_root;

    void setRoot(Entity entity, Entity root);
    void removeRoot(Entity entity);
    void removeTransform(Entity entity);
};

}    // namespace pivot::internal
