#include <pivot/ecs/Core/Data/value.hxx>

#include "pivot/pivot.hxx"

namespace pivot::ecs::data
{

RecordType Record::type() const
{
    PROFILE_FUNCTION();
    RecordType type;

    for (const auto &[key, value]: *this) { type.insert({key, value.type()}); }

    return type;
}

Type Value::type() const
{
    PROFILE_FUNCTION();
    return std::visit(
        [](const auto &value) {
            using type = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<type, Record>) {
                return Type{value.type()};
            } else {
                return Type{basic_type_representation<type>.value()};
            }
        },
        // FIXME: Remove the static_cast when GCC 12 releases
        static_cast<const Value::variant &>(*this));
}

bool List::operator==(const List &rhs) const { return items.size() == rhs.items.size(); }
auto List::operator<=>(const List &rhs) const { return items.size() <=> rhs.items.size(); }
}    // namespace pivot::ecs::data
