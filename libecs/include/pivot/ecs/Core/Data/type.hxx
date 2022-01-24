#pragma once

#include <glm/vec3.hpp>
#include <map>
#include <optional>
#include <ostream>
#include <string>
#include <variant>

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
};

struct Type;

/// The record type specifies the property names and types of a record
using RecordType = std::map<std::string, Type>;

/// A type in the pivot data model
struct Type : public std::variant<BasicType, RecordType> {
};

std::ostream &operator<<(std::ostream &stream, const BasicType &type);
std::ostream &operator<<(std::ostream &stream, const RecordType &type);
std::ostream &operator<<(std::ostream &stream, const Type &type);

template <typename T>
constexpr std::optional<BasicType> basic_type_representation;

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
