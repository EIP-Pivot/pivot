#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <map>
#include <optional>
#include <ostream>
#include <string>
#include <variant>

#include <pivot/ecs/Core/Data/asset.hxx>
#include <pivot/ecs/Core/Data/color.hxx>
#include <pivot/ecs/Core/Data/void.hxx>

namespace pivot::ecs::data
{

/// A basic type is a type that is not an aggregate of other type
enum class BasicType {
    /// The property is of type std::string
    String,

    /// The property is of type double
    Number,

    /// The property is of type int
    Integer,

    /// The property is of type bool
    Boolean,

    /// The property is of type glm::vec3
    Vec3,

    /// The property is of type glm::vec2
    Vec2,

    /// The property is of type Asset
    Asset,

    /// The property has no value
    Void,

    /// The property is of type Color
    Color,
};

struct Type;
struct Value;

/// The record type specifies the property names and types of a record
using RecordType = std::map<std::string, Type>;

/// A type in the pivot data model
struct Type : public std::variant<BasicType, RecordType> {
    using variant::variant;

    /// Converts to a human readable text representation of the type
    std::string toString() const;

    /// Creates a default value corresponding to this type
    data::Value defaultValue() const;
};

std::ostream &operator<<(std::ostream &stream, const BasicType &type);
std::ostream &operator<<(std::ostream &stream, const RecordType &type);
std::ostream &operator<<(std::ostream &stream, const Type &type);

template <typename T>
constexpr std::optional<BasicType> basic_type_representation = std::nullopt;

template <>
constexpr std::optional<BasicType> basic_type_representation<std::string> = BasicType::String;
template <>
constexpr std::optional<BasicType> basic_type_representation<double> = BasicType::Number;
template <>
constexpr std::optional<BasicType> basic_type_representation<int> = BasicType::Integer;
template <>
constexpr std::optional<BasicType> basic_type_representation<bool> = BasicType::Boolean;
template <>
constexpr std::optional<BasicType> basic_type_representation<glm::vec3> = BasicType::Vec3;
template <>
constexpr std::optional<BasicType> basic_type_representation<glm::vec2> = BasicType::Vec2;
template <>
constexpr std::optional<BasicType> basic_type_representation<Asset> = BasicType::Asset;
template <>
constexpr std::optional<BasicType> basic_type_representation<Color> = BasicType::Color;
template <>
constexpr std::optional<BasicType> basic_type_representation<Void> = BasicType::Void;

}    // namespace pivot::ecs::data

namespace std
{
template <>
struct variant_size<pivot::ecs::data::Type> : variant_size<pivot::ecs::data::Type::variant> {
};

template <std::size_t I>
struct variant_alternative<I, pivot::ecs::data::Type> : variant_alternative<I, pivot::ecs::data::Type::variant> {
};
}    // namespace std
