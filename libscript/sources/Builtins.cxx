
#include <iostream>

#include "pivot/script/Builtins.hxx"
#include "pivot/script/Exceptions.hxx"

#include <math.h>

namespace pivot::ecs::script::interpreter::builtins
{

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
                } else {
                    throw std::runtime_error("Code branch shouldn't execute.");
                }
            },
            static_cast<const data::Value::variant &>(param));
    }
    stream << std::endl;
    return data::Value();
}

data::Value builtin_loadScene(const std::vector<data::Value> &params, const BuiltinContext &context)
{
    return context.loadScene(std::get<std::string>(params.at(0)));
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

// Mathematical/Arithmetic operators -- end

}    // end of namespace pivot::ecs::script::interpreter::builtins
