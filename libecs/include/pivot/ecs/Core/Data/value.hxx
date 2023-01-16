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
 * - Entity (entity reference)
 */

/// Value containing an entity record and id
struct ScriptEntity {
    /// Data::Record of the entities components
    data::Record components;

    /// Entity id of the entity
    pivot::Entity entityId;

    bool operator<(const ScriptEntity &rhs) const { return entityId < rhs.entityId; }
    bool operator>(const ScriptEntity &rhs) const { return rhs.entityId < entityId; }
    bool operator<=(const ScriptEntity &rhs) const { return !(entityId > rhs.entityId); }
    bool operator>=(const ScriptEntity &rhs) const { return !(entityId < rhs.entityId); }
    bool operator==(const ScriptEntity &rhs) const { return entityId == rhs.entityId; }
    bool operator!=(const ScriptEntity &rhs) const { return !(entityId == rhs.entityId); }
};

/// Value containing a list
struct List {
    /// List of values
    std::vector<data::Value> items;

    bool operator<(const List &rhs) const { return items.size() < rhs.items.size(); }
    bool operator>(const List &rhs) const { return rhs.items.size() < items.size(); }
    bool operator<=(const List &rhs) const { return !(items.size() > rhs.items.size()); }
    bool operator>=(const List &rhs) const { return !(items.size() < rhs.items.size()); }
    bool operator==(const List &rhs) const { return items.size() == rhs.items.size(); }
    bool operator!=(const List &rhs) const { return !(items.size() == rhs.items.size()); }
};

struct Value : public std::variant<std::string, double, int, bool, glm::vec3, glm::vec2, Record, Asset, Color, Void,
                                   EntityRef, ScriptEntity, List> {
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
