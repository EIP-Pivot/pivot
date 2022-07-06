#pragma once

#include <glm/vec3.hpp>
#include <map>
#include <string>
#include <variant>

#include <pivot/ecs/Core/Data/type.hxx>

namespace pivot::ecs::data
{

struct Value;

/** \brief A record containg a mapping between property names and values
 *
 * This is equivalent to a JSON object
 */
struct Record : public std::map<std::string, Value> {
    using map::map;

    /// Returns the RecordType corresponding to this Record
    RecordType type() const;
};

/** \brief A value dynamically typed, which forms the basis of the pivot data model
 *
 * A value can be one of those type :
 * - Number (double)
 * - Integer (int)
 * - String (std::string)
 * - Record
 * - Vec3 (glm::vec3)
 * - Asset (name and path)
 * - Void (no value)
 */
struct Value : public std::variant<std::string, double, int, bool, glm::vec3, glm::vec2, Record, Asset, Void> {
    using variant::variant;

    /// Returns the Type corresponding to this Value
    Type type() const;

    /// Visits every simple datum in the value
    template <typename F>
    void visit_data(F f) const
    {
        std::visit(
            [&](const auto &datum) {
                using type = std::decay_t<decltype(datum)>;
                if constexpr (std::is_same_v<type, Record>) {
                    for (const auto &[_, value]: datum) { value.visit_data(f); }
                } else {
                    f(datum);
                }
            },
            static_cast<const Value::variant &>(*this));
    }
};

}    // namespace pivot::ecs::data

namespace std
{
template <>
struct variant_size<pivot::ecs::data::Value> : variant_size<pivot::ecs::data::Value::variant> {
};

template <std::size_t I>
struct variant_alternative<I, pivot::ecs::data::Value> : variant_alternative<I, pivot::ecs::data::Value::variant> {
};
}    // namespace std
