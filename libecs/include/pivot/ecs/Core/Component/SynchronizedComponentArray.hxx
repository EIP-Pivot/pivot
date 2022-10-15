#pragma once

#include <mutex>

#include "pivot/ecs/Core/Component/DenseComponentArray.hxx"
#include "pivot/pivot.hxx"

namespace pivot::ecs::component
{

/** \copybrief  DenseTypedComponentArray
 *
 * This child class of the DenseTypedComponentArray allow the array to be safely shared between thread.
 */
template <typename T, typename Mutex = std::mutex>
class SynchronizedTypedComponentArray : public DenseTypedComponentArray<T>
{
public:
    /// Creates a SynchronizedTypedComponentArray from the Description of its component
    using DenseTypedComponentArray<T>::DenseTypedComponentArray;

    /// \copydoc pivot::ecs::component::IComponentArray::getValueForEntity()
    std::optional<data::Value> getValueForEntity(Entity entity) const override
    {
        std::unique_lock lock(accessMutex);

        return DenseTypedComponentArray<T>::getValueForEntity(entity);
    }

    /// \copydoc pivot::ecs::component::IComponentArray::entityHasValue()
    bool entityHasValue(Entity entity) const override
    {
        std::unique_lock lock(accessMutex);

        return DenseTypedComponentArray<T>::entityHasValue(entity);
    }

    /// \copydoc pivot::ecs::component::IComponentArray::setValueForEntity()
    void setValueForEntity(Entity entity, std::optional<data::Value> value) override
    {
        std::unique_lock lock(accessMutex);

        return DenseTypedComponentArray<T>::setValueForEntity(entity, std::move(value));
    }

    /// \copydoc pivot::ecs::component::IComponentArray::maxEntity()
    Entity maxEntity() const override
    {
        std::unique_lock lock(accessMutex);
        return DenseTypedComponentArray<T>::maxEntity();
    }

    /// Manually lock the mutex when using the member access function.
    std::unique_lock<Mutex> lock() const { return std::unique_lock(accessMutex); }

    /// Manually lock the mutex when using the member access function.
    std::unique_lock<Mutex> try_lock() const { return std::unique_lock(accessMutex, std::try_to_lock); }

    /// Returns a mutable view into the components values. Some of those values can be nonsensical as the entity can
    /// miss this component.
    std::span<T> getData() override
    {
        pivotAssert(accessMutex.try_lock());
        return DenseTypedComponentArray<T>::getData();
    }

    /// Returns a constant view into the components values. Some of those values can be nonsensical as the entity can
    /// miss this component.
    std::span<const T> getData() const override
    {
        pivotAssert(accessMutex.try_lock());
        return DenseTypedComponentArray<T>::getData();
    }

    /// Returns the booleans specifying whether an an entity has the component
    const std::vector<bool> &getExistence() const override
    {
        pivotAssert(accessMutex.try_lock());
        return DenseTypedComponentArray<T>::getExistence();
    }

    /// Returns the vector containing the component data
    const std::vector<T> &getComponents() const override
    {
        pivotAssert(accessMutex.try_lock());
        return DenseTypedComponentArray<T>::getComponents();
    }

private:
    mutable Mutex accessMutex;
};
}    // namespace pivot::ecs::component
