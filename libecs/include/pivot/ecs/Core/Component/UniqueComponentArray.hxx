#pragma once

#include <set>
#include <stdexcept>

#include <pivot/pivot.hxx>

#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/array.hxx>

namespace pivot::ecs::component
{

/** \brief Storage for Tag components
 *
 * This IComponentArray is a DenseTypedComponentArray, but it additionally
 * checks that every entity has always a duplicate Tag, by throwing a
 * DuplicateEntityTag exception when a duplicated Tag is registered.
 */
template <Hashable T>
class UniqueComponentArray : public DenseTypedComponentArray<T>
{
public:
    /// Creates a UniqueComponentArray using the Description of T.
    UniqueComponentArray(Description d): DenseTypedComponentArray<T>(d) {}

    /// \copydoc pivot::ecs::component::IComponentArray::setValueForEntity()
    void setValueForEntity(Entity entity, std::optional<data::Value> value) override
    {

        std::optional<T> old_value = this->getEntity(entity);

        DenseTypedComponentArray<T>::setValueForEntity(entity, value);

        // Check component did not exist
        if (value.has_value()) {
            std::optional<T> new_value = this->getEntity(entity);
            if (m_unique_hash.contains(std::hash<T>()(new_value.value()))) {
                this->setEntity(entity, old_value);
                throw DuplicateComponent("TODO");
            }
        }

        // Remove old tag from tag set
        if (old_value.has_value()) { m_unique_hash.erase(std::hash<T>()(old_value.value())); }

        // Add new tag to tag set
        if (value.has_value()) {
            std::optional<T> new_value = this->getEntity(entity);
            m_unique_hash.insert({std::hash<T>()(new_value.value()), entity});
        }
    }

    /// Error thrown when a duplicate Tag is registered
    class DuplicateComponent : public std::logic_error
    {
    public:
        /// Creates a DuplicateComponent with the duplicated tag
        DuplicateComponent(const std::string &tag): std::logic_error("Duplicate entity tag: " + tag) {}
    };

protected:
    /// Map between hashes of component and entity id
    std::map<std::size_t, Entity> m_unique_hash;
};

}    // namespace pivot::ecs::component
