#include <pivot/ecs/Core/Data/value.hxx>

namespace pivot::ecs::data
{

RecordType Record::type() const
{
    RecordType type;

    for (const auto &[key, value]: *this) { type.insert({key, value.type()}); }

    return type;
}

OptionalType Optional::type() const
{
    if (this->has_value()) return OptionalType{this->value()};
    return OptionalType{};
}

Type Value::type() const
{
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
}    // namespace pivot::ecs::data
