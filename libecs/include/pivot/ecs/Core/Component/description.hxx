#pragma once

#include <any>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include <glm/vec3.hpp>

#include <pivot/ecs/Core/Data/type.hxx>

/** \brief Contains everything related to components creation and usage
 *
 * The components are the *data* part of the ECS. They store every property a
 * entity can have. They are not objects in the OOP sense. They should only be
 * used to represent data, and have no behaviour of their own, except for
 * maintaining themselves in a consistent state. A component should definitely
 * NOT every interact with the outside world.
 */
namespace pivot::ecs::component
{

class IComponentArray;

/** \brief Describes how to use a component
 *
 * A component is a simple data type. It can be defined at compile time as a C++
 * POD, or at runtime using any other mechanism, notably the scriting engine.
 *
 * Therefore a component data layout can not (always) be known at compile time.
 * A component is stored as an `std::any`, and the Description provides a way to
 * interact with this component.
 */
struct Description {

    /// The name of the component. It cannot be empty
    std::string name;

    /// The type of values storeds in this component
    data::Type type;

    /** \brief Type of the factory of the IComponentArray of the component
     *
     * This function is used to create an IComponentArray which can store values of the component associated to
     * entities.
     *
     * \param description The Description of the component
     */
    using CreateContainerType = std::unique_ptr<IComponentArray>(Description description);

    /// Functor creating an instance of the IComponentArray type associated with this component
    CreateContainerType &createContainer;

    /// Convenience method used to check if any error exist the component's definition
    void validate() const;

    /// Error returned when the validation of a description fails
    class ValidationError : public std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /// Compare descriptions.
    bool operator==(const Description &rhs) const;
};
}    // namespace pivot::ecs::component
