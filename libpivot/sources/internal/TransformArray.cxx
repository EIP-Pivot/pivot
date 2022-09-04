#include <cpplogger/Logger.hpp>

#include <pivot/internal/TransformArray.hxx>

using Transform = pivot::graphics::Transform;

namespace pivot::internal
{

void TransformArray::setValueForEntity(Entity entity, std::optional<ecs::data::Value> value)
{
    if (value.has_value()) {
        if (entity >= m_components.size()) {
            m_components.resize(entity + 1);
            m_component_exist.resize(entity + 1, false);
            m_reverse_root.resize(entity + 1, std::set<Entity>{});
        }

        Transform newTransform = this->parseValue(value.value());
        if (newTransform.root.is_empty()) {
            this->removeRoot(entity);
        } else {
            this->setRoot(entity, newTransform.root.ref);
        }

        Transform &entityTransform = m_components.at(entity);
        entityTransform.position = newTransform.position;
        entityTransform.rotation = newTransform.rotation;
        entityTransform.scale = newTransform.scale;
        m_component_exist.at(entity) = true;
    } else {
        this->removeTransform(entity);
    }
}

void TransformArray::setRoot(Entity entity, Entity root)
{
    Transform &transform = this->m_components.at(entity);

    // Handle the old root only if the entity exist. Otherwise the root value is
    // just some remaining value from a previous entity
    if (this->entityHasValue(entity)) {
        // If the old root is the same as the new root, no need to do anything
        if (!transform.root.is_empty() && transform.root.ref == root) { return; }

        // Otherwise, remove the old root
        this->removeRoot(entity);
    }

    // Check that the new root corresponds to an entity with a Transform component
    if (!this->entityHasValue(root)) {
        logger.warn() << "Cannot set transform root to entity without transform";
        transform.root = EntityRef::empty();
        return;
    }

    // Check that the new root is not the entity itself
    if (entity == root) {
        logger.warn() << "Cannot set transform root to self";
        transform.root = EntityRef::empty();
        return;
    }

    const Transform &rootTransform = this->m_components.at(root);

    // If the new root also has a root, the depth is larger than 1
    if (!rootTransform.root.is_empty()) { throw RootDepthExceeded(entity); }

    // If the entity is the root of other entities, the depth is larger than 1
    if (!m_reverse_root.at(entity).empty()) { throw RootDepthExceeded(*m_reverse_root.at(entity).begin()); }

    // Add root transform component to model matrix
    transform = transform.with_root(m_components.at(root));

    // Add new root
    transform.root.ref = root;

    // Add backlink to new root
    m_reverse_root.at(root).insert(entity);
}

void TransformArray::removeRoot(Entity entity)
{
    // If the entity has no transform, there is no root to remove
    if (!this->entityHasValue(entity)) { return; }

    Transform &transform = this->m_components.at(entity);

    // If the entity does not have a root, there is nothing to do
    if (transform.root.is_empty()) { return; }

    // Remove backlink to the root entity
    m_reverse_root.at(transform.root.ref).erase(entity);

    // Remove root transform component of model matrix
    transform = transform.remove_root(m_components.at(transform.root.ref));

    // Remove root
    transform.root = EntityRef::empty();
}

void TransformArray::removeTransform(Entity entity)
{
    // Nothing to do if the entity has no transform
    if (!this->entityHasValue(entity)) return;

    Transform &transform = m_components.at(entity);

    if (transform.root.is_empty()) {
        // If the entity has no root, remove all reverse roots
        for (auto dep_entity: m_reverse_root.at(entity)) { m_components.at(dep_entity).root = EntityRef::empty(); }
        m_reverse_root.at(entity).clear();
    } else {
        // If the entity has a root, remove the root
        this->removeRoot(entity);
    }

    m_component_exist.at(entity) = false;
}
}    // namespace pivot::internal
