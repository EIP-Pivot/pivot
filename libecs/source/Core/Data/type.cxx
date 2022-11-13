#include <magic_enum.hpp>
#include <sstream>

#include <pivot/ecs/Core/Data/type.hxx>
#include <pivot/ecs/Core/Data/value.hxx>

#include "pivot/pivot.hxx"

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
    PROFILE_FUNCTION();
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
                    case BasicType::EntityRef: return Value{EntityRef::empty()};
                    case BasicType::Vec2: return Value{glm::vec2{0, 0}};
                    case BasicType::Void: return Value{Void{}};
                    case BasicType::Color: return Value{Color{1, 1, 1, 1}};
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

bool Type::isSubsetOf(const Type &other) const
{
    PROFILE_FUNCTION();
    if (const RecordType *record = std::get_if<RecordType>(this)) {
        if (const RecordType *other_record = std::get_if<RecordType>(&other)) {
            for (auto &[key, subtype]: *record) {
                auto other_subtype = other_record->find(key);
                if (other_subtype == other_record->end()) { return false; }
                if (other_subtype->second != subtype) { return false; }
            }
            return true;
        }
    }
    return *this == other;
}
}    // namespace pivot::ecs::data
