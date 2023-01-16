
#include <iostream>

#include "pivot/script/Builtins.hxx"
#include "pivot/script/Exceptions.hxx"

#include <math.h>

namespace pivot::ecs::script::interpreter::builtins
{

data::Value builtin_selectCamera(const std::vector<data::Value> &params, const BuiltinContext &context)
{
    EntityRef entity = std::get<EntityRef>(params.at(0));
    if (!entity.is_empty()) { context.selectCamera(std::make_optional(entity.ref)); }
    return {data::Void{}};
}

data::Value builtin_isPressed(const std::vector<data::Value> &params, const BuiltinContext &context)
{
    return context.isKeyPressed(std::get<std::string>(params.at(0)));
}

data::Value builtin_print(const std::vector<data::Value> &params, const BuiltinContext &)
{
    return builtin_print_stream(params, std::cout);
}

data::Value builtin_print_stream(const std::vector<data::Value> &params, std::ostream &stream)
{
    bool first = true;
    for (const data::Value &param: params) {    // print has unlimited number of parameters
        std::visit(
            [&first, &stream](auto &value) {
                if (!first) { stream << " "; }
                first = false;

                using type = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<type, double> || std::is_same_v<type, std::string> ||
                              std::is_same_v<type, int>) {
                    stream << value;
                } else if constexpr (std::is_same_v<type, bool>) {
                    stream << std::boolalpha << value;
                } else if constexpr (std::is_same_v<type, pivot::ecs::data::Asset>) {
                    stream << "Asset(" << value.name << ")";
                } else if constexpr (std::is_same_v<type, glm::vec3>) {
                    stream << "vec3(" << value.x << "," << value.y << "," << value.z << ")";
                } else if constexpr (std::is_same_v<type, data::Color>) {
                    stream << "color(" << value.rgba[0] << "," << value.rgba[1] << "," << value.rgba[2] << ","
                           << value.rgba[3] << ")";
                } else if constexpr (std::is_same_v<type, pivot::EntityRef>) {
                    stream << "EntityRef(" << (value.is_empty() ? "EMPTY" : std::to_string(value.ref).c_str()) << ")";
                } else if constexpr (std::is_same_v<type, data::List>) {
                    stream << "List\t";
                    builtin_print_stream((std::get<data::List>((const data::Value &)value)).items, stream);
                } else {
                    throw std::runtime_error("Code branch shouldn't execute.");
                }
            },
            static_cast<const data::Value::variant &>(param));
    }
    stream << std::endl;
    return data::Value();
}

data::Value builtin_cos(const std::vector<data::Value> &params, const BuiltinContext &)
{
    return std::cos(std::get<double>(params.at(0)));
}

data::Value builtin_sin(const std::vector<data::Value> &params, const BuiltinContext &)
{
    return std::sin(std::get<double>(params.at(0)));
}

data::Value builtin_randint(const std::vector<data::Value> &params, const BuiltinContext &)
{
    int max = std::get<double>(params.at(0));
    double random_variable = std::rand() % max;
    return data::Value(random_variable);
}

data::Value builtin_power(const std::vector<data::Value> &params, const BuiltinContext &)
{
    return data::Value(pow(std::get<double>(params[0]), std::get<double>(params[1])));
}

data::Value builtin_sqrt(const std::vector<data::Value> &params, const BuiltinContext &)
{
    double rooted = std::get<double>(params[0]);
    if (rooted >= 0)
        return data::Value(sqrt(rooted));
    else {
        std::cerr << "ValueError: math domain error -- sqrt of a negative Number" << std::endl;
        throw(std::runtime_error("Code shouldn't execute."));
    }
}

data::Value builtin_abs(const std::vector<data::Value> &params, const BuiltinContext &)
{
    auto absVal = std::get<double>(params[0]) > 0 ? std::get<double>(params[0]) : -std::get<double>(params[0]);
    return data::Value(absVal);
}

data::Value builtin_not(const std::vector<data::Value> &params, const BuiltinContext &context)
{
    return (!std::get<bool>(params.at(0)));
}

data::Value builtin_createEntity(const std::vector<data::Value> &params, const BuiltinContext &context)
{
    std::pair<pivot::Entity, std::string> entityId = context.createEntity(std::get<std::string>(params.at(0)));

    data::ScriptEntity createdScriptEntity{data::Record{{"name", entityId.second}}, entityId.first};
    data::Value createdEntity = data::Value(createdScriptEntity);
    return createdEntity;
}

data::Value builtin_removeEntity(const std::vector<data::Value> &params, const BuiltinContext &context)
{
    context.removeEntity(std::get<std::string>(params.at(0)));
    return data::Value();
}

data::Value builtin_emitEvent(const std::vector<data::Value> &params, const BuiltinContext &context)
{
    std::cout << "Emitting event " << std::get<std::string>(params.at(0)) << std::endl;
    return data::Value();
}

data::Value builtin_addComponent(const std::vector<data::Value> &params, const BuiltinContext &context)
{
    return data::Value();
}

std::string removeTrailingZeroes(std::string str)
{    // Helper function until std::format is available in gcc
    str.erase(str.find_last_not_of('0') + 1, std::string::npos);
    str.erase(str.find_last_not_of('.') + 1, std::string::npos);
    return str;
}

data::Value builtin_toString(const std::vector<data::Value> &params, const BuiltinContext &)
{
    bool first = true;
    std::string result = "";
    for (const data::Value &param: params) {
        if (!first) { result += " "; };
        first = false;
        std::visit(
            [&result](auto &value) {
                using type = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<type, double> || std::is_same_v<type, int>) {
                    // result += std::format("{}", value);
                    result += removeTrailingZeroes(std::to_string(value));
                } else if constexpr (std::is_same_v<type, std::string>) {
                    // result += std::format("{}", value);
                    result += value;
                } else if constexpr (std::is_same_v<type, bool>) {
                    result += (value) ? "True" : "False";
                } else if constexpr (std::is_same_v<type, pivot::ecs::data::Asset>) {
                    // result += std::format("Asset({})", value.name);
                    result += "Asset(" + value.name + ")";
                } else if constexpr (std::is_same_v<type, glm::vec3>) {
                    // result += std::format("Vector3({},{},{})", value.x, value.y, value.z);
                    result += "Vector3(" + removeTrailingZeroes(std::to_string(value.x)) + "," +
                              removeTrailingZeroes(std::to_string(value.y)) + "," +
                              removeTrailingZeroes(std::to_string(value.z)) + ")";
                } else if constexpr (std::is_same_v<type, glm::vec2>) {
                    // result += std::format("Vector2({},{})", value.x, value.y);
                    result += "Vector2(" + removeTrailingZeroes(std::to_string(value.x)) + "," +
                              removeTrailingZeroes(std::to_string(value.y)) + ")";
                } else if constexpr (std::is_same_v<type, data::Color>) {
                    // result += std::format("Color({},{},{},{})", value.rgba[0], value.rgba[1], value.rgba[2],
                    // value.rgba[3]);
                    result += "Color(" + removeTrailingZeroes(std::to_string(value.rgba[0])) + "," +
                              removeTrailingZeroes(std::to_string(value.rgba[1])) + "," +
                              removeTrailingZeroes(std::to_string(value.rgba[2])) + "," +
                              removeTrailingZeroes(std::to_string(value.rgba[3])) + ")";
                } else {
                    throw std::runtime_error("Code branch shouldn't execute.");
                }
            },
            static_cast<const data::Value::variant &>(param));
    }
    return data::Value(result);
}

// Builtin binary (which take two operands) operators
// TOOD: find a better (template?) solution to handle these cases

// Relational operators -- start

// TODO : try function blocks
template <>
data::Value builtin_operator<Operator::GreaterThan>(const data::Value &left, const data::Value &right)
try {
    if (std::get<data::BasicType>(left.type()) == data::BasicType::Number &&
        std::get<data::BasicType>(right.type()) == data::BasicType::Number)
        return data::Value(std::get<double>(left) > std::get<double>(right));    // perform arithmetic comparison
    else if (std::get<data::BasicType>(left.type()) == data::BasicType::String &&
             std::get<data::BasicType>(right.type()) == data::BasicType::String)
        // auto &strLeft = std::get_if<std::string>(left);
        // if (!strLeft || !strRight) // std::get_if()
        // 	throw();
        return data::Value(std::get<std::string>(left) >
                           std::get<std::string>(right));    // perform alphabetical comparison
    else {                                                   // unsupported
        logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
        throw InvalidOperation("Invalid greater than '>' operator between these types.");
    }
} catch (const std::bad_variant_access &) {
    logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
    throw InvalidOperation("Invalid greater than '>' operator between these types.");
}

template <>
data::Value builtin_operator<Operator::GreaterOrEqual>(const data::Value &left, const data::Value &right)
{    // greater than or equal to operator '>='
    try {
        if (std::get<data::BasicType>(left.type()) == data::BasicType::Number &&
            std::get<data::BasicType>(right.type()) == data::BasicType::Number)
            return data::Value(std::get<double>(left) >= std::get<double>(right));    // perform arithmetic comparison
        else if (std::get<data::BasicType>(left.type()) == data::BasicType::String &&
                 std::get<data::BasicType>(right.type()) == data::BasicType::String)
            return data::Value(std::get<std::string>(left) >=
                               std::get<std::string>(right));    // perform alphabetical comparison
        else {                                                   // unsupported
            logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
            throw InvalidOperation("Invalid greather than or equal to '>=' operator between these types.");
        }
    } catch (const std::bad_variant_access &) {
        logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
        throw InvalidOperation("Invalid greather than or equal to '>=' operator between these types.");
    }
}
template <>
data::Value builtin_operator<Operator::LowerThan>(const data::Value &left, const data::Value &right)
{    // lower than operator '<'
    try {
        if (std::get<data::BasicType>(left.type()) == data::BasicType::Number &&
            std::get<data::BasicType>(right.type()) == data::BasicType::Number)
            return data::Value(std::get<double>(left) < std::get<double>(right));    // perform arithmetic comparison
        else if (std::get<data::BasicType>(left.type()) == data::BasicType::String &&
                 std::get<data::BasicType>(right.type()) == data::BasicType::String)
            return data::Value(std::get<std::string>(left) <
                               std::get<std::string>(right));    // perform alphabetical comparison
        else {

            logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
            throw InvalidOperation("Invalid lower than '<' operator between these types.");
        }    // unsupported
    } catch (const std::bad_variant_access &) {
        logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
        throw InvalidOperation("Invalid lower than '<' operator between these types.");
    }
}
template <>
data::Value builtin_operator<Operator::LowerOrEqual>(const data::Value &left, const data::Value &right)
{    // lower than or equal to operator '<='
    try {
        if (std::get<data::BasicType>(left.type()) == data::BasicType::Number &&
            std::get<data::BasicType>(right.type()) == data::BasicType::Number)
            return data::Value(std::get<double>(left) <= std::get<double>(right));    // perform arithmetic comparison
        else if (std::get<data::BasicType>(left.type()) == data::BasicType::String &&
                 std::get<data::BasicType>(right.type()) == data::BasicType::String)
            return data::Value(std::get<std::string>(left) <=
                               std::get<std::string>(right));    // perform alphabetical comparison
        else {                                                   // unsupported
            logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
            throw InvalidOperation("Invalid lower than or equal to '<=' operator between these types.");
        }
    } catch (const std::bad_variant_access &) {
        logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
        throw InvalidOperation("Invalid lower than or equal to '<=' operator between these types.");
    }
}

// Relational operators -- end

// Mathematical/Arithmetic operators -- start
template <>
data::Value builtin_operator<Operator::Addition>(const data::Value &left, const data::Value &right)
{    // additive operator '+'
    try {
        if (std::get<data::BasicType>(left.type()) == data::BasicType::Number &&
            std::get<data::BasicType>(right.type()) == data::BasicType::Number)
            return data::Value(std::get<double>(left) + std::get<double>(right));    // perform arithmetic addition
        else if (std::get<data::BasicType>(left.type()) == data::BasicType::String &&
                 std::get<data::BasicType>(right.type()) == data::BasicType::String)
            return data::Value(std::get<std::string>(left) +
                               std::get<std::string>(right));    // perform string concatenation
        else if (std::get<data::BasicType>(left.type()) == data::BasicType::Vec3 &&
                 std::get<data::BasicType>(right.type()) == data::BasicType::Vec3)
            return data::Value(std::get<glm::vec3>(left) + std::get<glm::vec3>(right));    // let glm add the vectors
        else {                                                                             // unsupported
            logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
            throw InvalidOperation("Invalid addition '+' operator between these types.");
        }
    } catch (const std::bad_variant_access &) {
        logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
        throw InvalidOperation("Invalid addition '+' operator between these types.");
    }
}
template <>
data::Value builtin_operator<Operator::Substraction>(const data::Value &left, const data::Value &right)
{    // additive operator '-'
    try {
        if (std::get<data::BasicType>(left.type()) == data::BasicType::Number &&
            std::get<data::BasicType>(right.type()) == data::BasicType::Number)
            return data::Value(std::get<double>(left) - std::get<double>(right));    // perform arithmetic substraction
        else if (std::get<data::BasicType>(left.type()) == data::BasicType::Vec3 &&
                 std::get<data::BasicType>(right.type()) == data::BasicType::Vec3)
            return data::Value(std::get<glm::vec3>(left) -
                               std::get<glm::vec3>(right));    // let glm substract the vectors
        else {                                                 // unsupported
            logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
            throw InvalidOperation("Invalid substraction '-' operator between these types.");
        }
    } catch (const std::bad_variant_access &) {
        logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
        throw InvalidOperation("Invalid substraction '-' operator between these types.");
    }
}
template <>
data::Value builtin_operator<Operator::Multiplication>(const data::Value &left, const data::Value &right)
{    // multiplication operator '*'
    try {
        if (std::get<data::BasicType>(left.type()) == data::BasicType::Number &&
            std::get<data::BasicType>(right.type()) == data::BasicType::Number) {
            return data::Value(std::get<double>(left) *
                               std::get<double>(right));    // perform arithmetic multiplication
        } else if (std::get<data::BasicType>(left.type()) == data::BasicType::Vec3 &&
                   std::get<data::BasicType>(right.type()) == data::BasicType::Vec3) {
            return data::Value(std::get<glm::vec3>(left) *
                               std::get<glm::vec3>(right));    // let glm multiply the vectors
        } else if (std::get<data::BasicType>(left.type()) == data::BasicType::Number &&
                   std::get<data::BasicType>(right.type()) == data::BasicType::Vec3) {
            glm::vec3 result = std::get<glm::vec3>(right);
            double factor = std::get<double>(left);
            result.x *= factor;
            result.y *= factor;
            result.z *= factor;
            return data::Value(result);
        } else if (std::get<data::BasicType>(left.type()) == data::BasicType::Vec3 &&
                   std::get<data::BasicType>(right.type()) == data::BasicType::Number) {
            glm::vec3 result = std::get<glm::vec3>(left);
            double factor = std::get<double>(right);
            result.x *= factor;
            result.y *= factor;
            result.z *= factor;
            return data::Value(result);
        } else if (std::get<data::BasicType>(left.type()) == data::BasicType::Integer &&
                   std::get<data::BasicType>(right.type()) == data::BasicType::Vec3) {
            glm::vec3 result = std::get<glm::vec3>(right);
            int factor = std::get<int>(left);
            result.x *= factor;
            result.y *= factor;
            result.z *= factor;
            return data::Value(result);
        } else if (std::get<data::BasicType>(left.type()) == data::BasicType::Vec3 &&
                   std::get<data::BasicType>(right.type()) == data::BasicType::Integer) {
            glm::vec3 result = std::get<glm::vec3>(left);
            int factor = std::get<int>(right);
            result.x *= factor;
            result.y *= factor;
            result.z *= factor;
            return data::Value(result);
        } else {    // unsupported
            logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
            throw InvalidOperation("Invalid multiplication '*' operator between these types.");
        }
    } catch (const std::bad_variant_access &) {
        logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
        throw InvalidOperation("Invalid multiplication '*' operator between these types.");
    }
}
template <>
data::Value builtin_operator<Operator::Divison>(const data::Value &left, const data::Value &right)
{    // division operator '/'
    try {
        if (std::get<data::BasicType>(left.type()) == data::BasicType::Number &&
            std::get<data::BasicType>(right.type()) == data::BasicType::Number) {
            if (std::get<double>(right) == 0.0) {    // handle div by zero
                logger.err("ERROR") << " by '" << std::get<double>(left) << "' and '0'";
                throw InvalidOperation("Cannot divide by zero.");
            }
            return data::Value(std::get<double>(left) / std::get<double>(right));    // perform arithmetic division
        } else if (std::get<data::BasicType>(left.type()) == data::BasicType::Vec3 &&
                   std::get<data::BasicType>(right.type()) == data::BasicType::Vec3)
            return data::Value(
                std::get<glm::vec3>(left) /
                std::get<glm::vec3>(right));    // let glm divide the vectors TODO: check it throws on div by zero
        else {                                  // unsupported
            logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
            throw InvalidOperation("Invalid division '/' operator between these types.");
        }
    } catch (const std::bad_variant_access &) {
        logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
        throw InvalidOperation("Invalid division '/' operator between these types.");
    }
}
template <>
data::Value builtin_operator<Operator::Modulo>(const data::Value &left, const data::Value &right)
{    // modulo operator '%'
    try {
        if (std::get<data::BasicType>(left.type()) == data::BasicType::Integer &&
            std::get<data::BasicType>(right.type()) == data::BasicType::Integer) {
            if (std::get<int>(right) == 0) {    // handle modulo by zero
                logger.err("ERROR") << " by '" << std::get<int>(left) << "' and '0'";
                throw InvalidOperation("Cannot modulo by zero.");
            }
            return data::Value(std::get<int>(left) % std::get<int>(right));    // perform arithmetic modulo
        } else {                                                               // unsupported
            logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
            throw InvalidOperation("Invalid modulo '%' operator between these types.");
        }
    } catch (const std::bad_variant_access &) {
        logger.err("ERROR") << " by '" << left.type().toString() << "' and '" << right.type().toString() << "'";
        throw InvalidOperation("Invalid modulo '%' operator between these types.");
    }
}

data::Value builtin_vec3(const std::vector<data::Value> &params, const BuiltinContext &)
{
    return {glm::vec3{std::get<double>(params.at(0)), std::get<double>(params.at(1)), std::get<double>(params.at(2))}};
}

data::Value builtin_color(const std::vector<data::Value> &params, const BuiltinContext &)
{
    return {data::Color{.rgba = {(float)std::get<double>(params.at(0)), (float)std::get<double>(params.at(1)),
                                 (float)std::get<double>(params.at(2)), (float)std::get<double>(params.at(3))}}};
}

data::Value builtin_list(const std::vector<data::Value> &params, const BuiltinContext &)
{
    data::List r;

    for (const data::Value &param: params) { r.items.push_back(param); }
    return {r};
}

data::Value builtin_at(const std::vector<data::Value> &params, const BuiltinContext &)
{
    size_t index = (size_t)(std::get<double>(params.at(1)));
    const data::List &list = std::get<data::List>(params.at(0));
    if (index >= list.items.size()) {
        logger.err("ERROR") << " by index 'at(" << index << ")' and size '" << list.items.size() << "'";
        throw InvalidOperation("Index out of list range.");
    }
    return list.items.at(index);
}

data::Value builtin_len(const std::vector<data::Value> &params, const BuiltinContext &)
{
    return data::Value{(double)std::get<data::List>(params.at(0)).items.size()};
}

data::Value builtin_remove(const std::vector<data::Value> &params, const BuiltinContext &)
{
    size_t index = (size_t)(std::get<double>(params.at(1)));
    const data::List &list = std::get<data::List>(params.at(0));
    if (index >= list.items.size()) {
        logger.err("ERROR") << " by index 'at(" << index << ")' and size '" << list.items.size() << "'";
        throw InvalidOperation("Index out of list range.");
    }
    data::List r;
    for (size_t i = 0; i < list.items.size(); i++)
        if (i != index) r.items.push_back(data::Value{list.items.at(i)});
    return data::Value{r};
}

data::Value builtin_push(const std::vector<data::Value> &params, const BuiltinContext &)
{
    const data::List &list = std::get<data::List>(params.at(0));
    data::List r;
    for (const data::Value &v: list.items) r.items.push_back(v);
    for (size_t i = 1; i < params.size(); i++) r.items.push_back(params.at(i));
    return data::Value{r};
}

// Mathematical/Arithmetic operators -- end

}    // end of namespace pivot::ecs::script::interpreter::builtins
