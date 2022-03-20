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
    struct ComponentCombination {
    public:
        /// Default constructor
        ComponentCombination(ArrayCombination &intersection, Entity entity): intersection(intersection), entity(entity)
        {
        }

        /** \brief Accesses one of the components on the entity.
         *
         * The order is the same as in the ArrayCombination.
         */
        ComponentRef operator[](std::size_t i) { return {intersection.m_arrays[i], entity}; }

        /// Compare two ComponentCombonations
        bool operator==(ComponentCombination other) const
        {
            return &this->intersection == &other.intersection && this->entity == other.entity;
        }

        /// Checks that the entity has a component value for each component array
        bool isValid() const { return intersection.entityHasValue(entity); }

        /// All the component arrays of the combination
        ArrayCombination &intersection;

        /// The entity number of the combination
        Entity entity;
    };

    /// Iterator over all entity having components in each array
    class iterator
    {
    public:
        /// Begin constructor
        explicit iterator(ArrayCombination &intersection)
            : m_max_entity(intersection.maxEntity()), m_combination(intersection, 0)
        {
            if (!m_combination.isValid()) { goToNextValidEntity(); }
        }
        /// End constructor
        explicit iterator(ArrayCombination &intersection, bool end)
            : m_max_entity(intersection.maxEntity()), m_combination(intersection, m_max_entity + 1)
        {
        }

        /// @cond
        iterator &operator++()
        {
            goToNextValidEntity();
            return *this;
        }
        iterator operator++(int)
        {
            iterator retval = *this;
            ++(*this);
            return retval;
        }
        bool operator==(iterator other) const { return m_combination == other.m_combination; }
        const ComponentCombination &operator*() const { return m_combination; }
        ComponentCombination &operator*() { return m_combination; }
        const ComponentCombination *operator->() const { return &m_combination; }
        /// @endcond

    private:
        void goToNextValidEntity()
        {
            Entity &entity = m_combination.entity;
            while (entity <= m_max_entity) {
                entity++;
                if (m_combination.isValid()) { return; }
            }
        }

        Entity m_max_entity;
        ComponentCombination m_combination;
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
