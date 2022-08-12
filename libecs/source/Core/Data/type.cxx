#include <magic_enum.hpp>
#include <sstream>

#include <pivot/ecs/Core/Data/type.hxx>
#include <pivot/ecs/Core/Data/value.hxx>

namespace pivot::ecs::data
{

std::ostream &operator<<(std::ostream &stream, const BasicType &type)
{
    return stream << "BasicType::" << magic_enum::enum_name(type);
}

std::ostream &operator<<(std::ostream &stream, const RecordType &type)
{
    bool first = true;
    stream << "Record {";
    for (const auto &[key, value]: type) {
        if (!first) { stream << ", "; }
        stream << "{\"" << key << "\", " << value << "}";
        first = false;
    }
    return stream << "}";
}

std::ostream &operator<<(std::ostream &stream, const Type &type)
{
    stream << "Type { ";
    // FIXME: Remove the static_cast when GCC 12 releases
    std::visit([&](const auto &value) { stream << value; }, static_cast<const Type::variant &>(type));
    return stream << " }";
}

std::string Type::toString() const
{
    std::ostringstream oss;
    oss << *this;
    return oss.str();
}

data::Value Type::defaultValue() const
{
    return std::visit(
        [](const auto &type) {
            using type_type = std::decay_t<decltype(type)>;
            if constexpr (std::is_same_v<type_type, BasicType>) {
                switch (type) {
                    case BasicType::String: return Value{""};
                    case BasicType::Number: return Value{0.0};
                    case BasicType::Integer: return Value{0};
                    case BasicType::Boolean: return Value{false};
                    case BasicType::Vec3: return Value{glm::vec3{0, 0, 0}};
                    case BasicType::Asset: return Value{Asset{""}};
                    case BasicType::Void: return Value{Void{}};
                    case BasicType::Entity: return Value{EntityRef::empty()};
                    default: throw std::runtime_error("Unknown basic type");
                }
            } else {
                Record record;
                for (auto &[key, subtype]: type) { record.insert({key, subtype.defaultValue()}); }
                return Value{record};
            }
        },
        static_cast<const Type::variant &>(*this));
}
}    // namespace pivot::ecs::data
