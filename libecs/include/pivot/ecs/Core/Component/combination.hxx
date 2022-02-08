#pragma once

#include <algorithm>
#include <pivot/ecs/Core/Component/ref.hxx>

namespace pivot::ecs::component
{

/** \brief Combination of component arrays
 *
 * This class is a range which allows iterating over the component values
 * for each entity which is present in every array.
 */
class ArrayCombination
{
public:
    /// Default constructor
    ArrayCombination(std::vector<std::reference_wrapper<IComponentArray>> arrays): m_arrays(arrays) {}

    /** \brief One combination of components on an entity
     *
     * This allows access to all components of the entity as a ref.
     */
    class ComponentCombination
    {
    public:
        /// Default constructor
        ComponentCombination(ArrayCombination &intersection, Entity entity)
            : m_intersection(intersection), m_entity(entity)
        {
        }

        /** \brief Accesses one of the components on the entity.
         *
         * The order is the same as in the ArrayCombination.
         */
        ComponentRef operator[](std::size_t i) { return {m_intersection.m_arrays[i], m_entity}; }

    private:
        ArrayCombination &m_intersection;
        Entity m_entity;
    };

    /// Iterator over all entity having components in each array
    class iterator : std::iterator<std::input_iterator_tag, ComponentCombination>
    {
    public:
        /// Begin constructor
        explicit iterator(ArrayCombination &intersection)
            : m_intersection(intersection), m_max_entity(intersection.maxEntity()), m_entity(0)
        {
        }
        /// End constructor
        explicit iterator(ArrayCombination &intersection, bool end)
            : m_intersection(intersection), m_max_entity(intersection.maxEntity()), m_entity(m_max_entity + 1)
        {
        }

        /// @cond
        iterator &operator++()
        {
            while (m_entity <= m_max_entity) {
                m_entity++;
                if (m_intersection.entityHasValue(m_entity)) { break; }
            }
            return *this;
        }
        iterator operator++(int)
        {
            iterator retval = *this;
            ++(*this);
            return retval;
        }
        bool operator==(iterator other) const
        {
            return m_entity == other.m_entity && &m_intersection == &other.m_intersection;
        }
        bool operator!=(iterator other) const { return !(*this == other); }
        value_type operator*() const { return ComponentCombination(m_intersection, m_entity); }
        value_type operator->() const { return **this; }
        /// @endcond

    private:
        ArrayCombination &m_intersection;
        Entity m_max_entity;
        Entity m_entity;
    };

    /// Begin iterator
    iterator begin() { return iterator(*this); }
    /// End iterator
    iterator end() { return iterator(*this, true); }

private:
    Entity maxEntity() const;
    bool entityHasValue(Entity entity) const;

    std::vector<std::reference_wrapper<IComponentArray>> m_arrays;
};
}    // namespace pivot::ecs::component
