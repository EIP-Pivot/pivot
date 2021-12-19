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

    /** \brief A property (member) of a component
     *
     * A component is composed of multiple named and typed properties. For now
     * components can only contain a flat list of named properties and their
     * corresponding types, meaning they cannot be nested.
     *
     * All the property names of a component must be unique, and no property is
     * allowed to have an empty name.
     */
    struct Property {
        /// Type of a property. It must always be a valid value of the Type enum.
        enum class Type {
            /// The property is of type std::string
            STRING,

            /// The property is of type int
            NUMBER,

            /// Unused for now
            ASSET,

            /// The property is of type glm::vec3
            VEC3,
        };

        // TODO: Store the type with the value

        /// The type of component's property value
        using ValueType = std::variant<int, std::string, glm::vec3>;

        /// Name of a property
        std::string name;

        /// The type of the property
        Type type;
    };

    /// The name of the component. It cannot be empty
    std::string name;

    /// The list of properties of the component.
    std::vector<Property> properties;

    /** \brief Type of the property getter of a component
     *
     * This function is used to access a named property of a component.
     *
     * \param component The value of the component. It must be a value created with CreateContainerType.
     * \param property The name of the property to access
     */
    using GetPropertyType = Property::ValueType(std::any component, std::string property);

    /** \brief Type of the property setter of a component
     *
     * This function is used to set the value of a named property of a component.
     *
     * \param component The value of the component. It must be a value created with CreateContainerType.
     * \param property The name of the property to access.
     * \param value The value to put in the property. It must be of the same type as described in Property::Type
     */
    using SetPropertyType = void(std::any &component, std::string property, Property::ValueType value);

    /** \brief Type of the factory of the component
     *
     * This function is used to create a component, based on values of its properties.
     *
     * \param properties A std::map containing all the properties of the new component and their value.
     */
    using CreateType = std::any(std::map<std::string, Property::ValueType> properties);

    /** \brief Type of the factory of the IComponentArray of the component
     *
     * This function is used to create an IComponentArray which can store values of the component associated to
     * entities.
     *
     * \param description The Description of the component
     */
    using CreateContainerType = std::unique_ptr<IComponentArray>(Description description);

    /// Functor allowing to retrieve the value of a property of the component
    GetPropertyType &getProperty;
    /// Functor allowing to set the value of a property of the component
    SetPropertyType &setProperty;
    /// Functor creating a new instance of the component using the provided property and value list
    CreateType &create;
    /// Functor creating an instance of the IComponentArray type associated with this component
    CreateContainerType &createContainer;

    /// Convenience method used to check if any error exist the component's definition
    void validate() const;

    /// Error returned when the validation of a description fails
    class ValidationError : public std::logic_error
    {
        using std::logic_error::logic_error;
    };
};
}    // namespace pivot::ecs::component
