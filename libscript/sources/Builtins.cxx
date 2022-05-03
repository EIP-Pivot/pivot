#include "pivot/script/Builtins.hxx"
#include "Logger.hpp"
#include "pivot/script/Exceptions.hxx"

#include <iostream>

namespace pivot::ecs::script::interpreter::builtins
{

data::Value builtin_isPressed(const std::vector<data::Value> &params)
{
    // TODO : find a way to pass Window * as parameter
    return data::Value(true);
}

data::Value builtin_print(const std::vector<data::Value> &params)
{
    for (const data::Value &param: params) {    // print has unlimited number of parameters
        data::BasicType varType = std::get<data::BasicType>(param.type());
        switch (varType) {
            case data::BasicType::Number: std::cout << std::get<double>(param) << " "; break;
            case data::BasicType::String: std::cout << std::get<std::string>(param) << " "; break;
            case data::BasicType::Boolean: std::cout << (std::get<bool>(param) ? "true" : "false") << " "; break;
            default: throw(std::runtime_error("Code branch shouldn't execute."));
        }
    }
    std::cout << "\n";
    return data::Value();
}

// Builtin binary (which take two operands) operators
// TOOD: find a better (template?) solution to handle these cases

// Relational operators -- start

template <>
data::Value builtin_operator<Operator::Equal>(const data::Value &left, const data::Value &right)
{    // equal to operator '=='
    try {
        if (std::get<data::BasicType>(left.type()) == data::BasicType::Number &&
            std::get<data::BasicType>(right.type()) == data::BasicType::Number)
            return data::Value(std::get<double>(left) == std::get<double>(right));    // perform arithmetic comparison
        else if (std::get<data::BasicType>(left.type()) == data::BasicType::String &&
                 std::get<data::BasicType>(right.type()) == data::BasicType::String)
            return data::Value(std::get<std::string>(left) ==
                               std::get<std::string>(right));    // perform alphabetical comparison
        else if (std::get<data::BasicType>(left.type()) == data::BasicType::Boolean &&
                 std::get<data::BasicType>(right.type()) == data::BasicType::Boolean)
            return data::Value(std::get<bool>(left) == std::get<bool>(right));    // perform arithmetic comparison
        else if (std::get<data::BasicType>(left.type()) == data::BasicType::Vec3 &&
                 std::get<data::BasicType>(right.type()) == data::BasicType::Vec3)
            return data::Value(std::get<glm::vec3>(left) == std::get<glm::vec3>(right));    // let glm do the comparison
        else                                                                                // unsupported
            throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                                   "Invalid equal to '==' operator between these types.");
    } catch (std::bad_variant_access e) {
        throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                               "Invalid equal to '==' operator between these types.");
    }
}
template <>
data::Value builtin_operator<Operator::NotEqual>(const data::Value &left, const data::Value &right)
{    // not equal to operator '!='
    try {
        if (std::get<data::BasicType>(left.type()) == data::BasicType::Number &&
            std::get<data::BasicType>(right.type()) == data::BasicType::Number)
            return data::Value(std::get<double>(left) != std::get<double>(right));    // perform arithmetic comparison
        else if (std::get<data::BasicType>(left.type()) == data::BasicType::String &&
                 std::get<data::BasicType>(right.type()) == data::BasicType::String)
            return data::Value(std::get<std::string>(left) !=
                               std::get<std::string>(right));    // perform alphabetical comparison
        else if (std::get<data::BasicType>(left.type()) == data::BasicType::Boolean &&
                 std::get<data::BasicType>(right.type()) == data::BasicType::Boolean)
            return data::Value(std::get<bool>(left) != std::get<bool>(right));    // perform arithmetic comparison
        else if (std::get<data::BasicType>(left.type()) == data::BasicType::Vec3 &&
                 std::get<data::BasicType>(right.type()) == data::BasicType::Vec3)
            return data::Value(std::get<glm::vec3>(left) != std::get<glm::vec3>(right));    // let glm do the comparison
        else                                                                                // unsupported
            throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                                   "Invalid not equal to '!=' operator between these types.");
    } catch (InvalidException e) {
        throw e;
    } catch (std::bad_variant_access e) {
        throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                               "Invalid not equal to '!=' operator between these types.");
    }
}
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
    else                                                     // unsupported
        throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                               "Invalid greater than '>' operator between these types.");
} catch (const std::bad_variant_access &) {
    throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                           "Invalid greater than '>' operator between these types.");
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
        else                                                     // unsupported
            throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                                   "Invalid greater than or equal to '>=' operator between these types.");
    } catch (InvalidException e) {
        throw e;
    } catch (std::bad_variant_access e) {
        throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                               "Invalid greater than or equal to '>=' operator between these types.");
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
        else                                                     // unsupported
            throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                                   "Invalid lower than '<' operator between these types.");
    } catch (InvalidException e) {
        throw e;
    } catch (std::bad_variant_access e) {
        throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                               "Invalid lower than '<' operator between these types.");
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
        else                                                     // unsupported
            throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                                   "Invalid lower than or equal to '<=' operator between these types.");
    } catch (InvalidException e) {
        throw e;
    } catch (std::bad_variant_access e) {
        throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                               "Invalid lower than or equal to '<=' operator between these types.");
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
        else                                                                               // unsupported
            throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                                   "Invalid addition '+' operator between these types.");
    } catch (InvalidException e) {
        throw e;
    } catch (std::bad_variant_access e) {
        throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                               "Invalid addition '+' operator between these types.");
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
        else                                                   // unsupported
            throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                                   "Invalid substraction '-' operator between these types.");
    } catch (InvalidException e) {
        throw e;
    } catch (std::bad_variant_access e) {
        throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                               "Invalid substraction '-' operator between these types.");
    }
}
template <>
data::Value builtin_operator<Operator::Multiplication>(const data::Value &left, const data::Value &right)
{    // multiplication operator '*'
    try {
        if (std::get<data::BasicType>(left.type()) == data::BasicType::Number &&
            std::get<data::BasicType>(right.type()) == data::BasicType::Number)
            return data::Value(std::get<double>(left) *
                               std::get<double>(right));    // perform arithmetic multiplication
        else if (std::get<data::BasicType>(left.type()) == data::BasicType::Vec3 &&
                 std::get<data::BasicType>(right.type()) == data::BasicType::Vec3)
            return data::Value(std::get<glm::vec3>(left) *
                               std::get<glm::vec3>(right));    // let glm multiply the vectors
        else                                                   // unsupported
            throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                                   "Invalid multiplication '*' operator between these types.");
    } catch (InvalidException e) {
        throw e;
    } catch (std::bad_variant_access e) {
        throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                               "Invalid multiplication '*' operator between these types.");
    }
}
template <>
data::Value builtin_operator<Operator::Divison>(const data::Value &left, const data::Value &right)
{    // division operator '/'
    try {
        if (std::get<data::BasicType>(left.type()) == data::BasicType::Number &&
            std::get<data::BasicType>(right.type()) == data::BasicType::Number) {
            if (std::get<double>(right) == 0.0)    // handle div by zero
                throw InvalidException("DivByZero", (std::to_string(std::get<double>(left)) + " / 0").c_str(),
                                       "Cannot divide by zero.");
            return data::Value(std::get<double>(left) / std::get<double>(right));    // perform arithmetic division
        } else if (std::get<data::BasicType>(left.type()) == data::BasicType::Vec3 &&
                   std::get<data::BasicType>(right.type()) == data::BasicType::Vec3)
            return data::Value(
                std::get<glm::vec3>(left) /
                std::get<glm::vec3>(right));    // let glm divide the vectors TODO: check it throws on div by zero
        else                                    // unsupported
            throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                                   "Invalid division '-' operator between these types.");
    } catch (InvalidException e) {
        throw e;
    } catch (std::bad_variant_access e) {
        throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                               "Invalid division '-' operator between these types.");
    }
}
template <>
data::Value builtin_operator<Operator::Modulo>(const data::Value &left, const data::Value &right)
{    // modulo operator '%'
    try {
        if (std::get<data::BasicType>(left.type()) == data::BasicType::Integer &&
            std::get<data::BasicType>(right.type()) == data::BasicType::Integer) {
            if (std::get<int>(right) == 0)    // handle modulo by zero
                throw InvalidException("DivByZero", (std::to_string(std::get<int>(left)) + " / 0").c_str(),
                                       "Cannot modulo by zero.");
            return data::Value(std::get<int>(left) % std::get<int>(right));    // perform arithmetic modulo
        } else                                                                 // unsupported
            throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                                   "Invalid modulo '%' operator between these types.");
    } catch (InvalidException e) {
        throw e;
    } catch (std::bad_variant_access e) {
        throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                               "Invalid modulo '%' operator between these types.");
    }
}

// Mathematical/Arithmetic operators -- end

// Logical operators -- start

template <>
data::Value builtin_operator<Operator::LogicalAnd>(const data::Value &left, const data::Value &right)
{    // logical operator '&&' AND
    try {
        if (std::get<data::BasicType>(left.type()) == data::BasicType::Boolean &&
            std::get<data::BasicType>(right.type()) == data::BasicType::Boolean)
            return data::Value(std::get<bool>(left) && std::get<bool>(right));    // perform logical AND
        else                                                                      // unsupported
            throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                                   "Invalid Logical AND '&&' operator between these types.");
    } catch (InvalidException e) {
        throw e;
    } catch (std::bad_variant_access e) {
        throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                               "Invalid Logical AND '&&' operator between these types.");
    }
}
template <>
data::Value builtin_operator<Operator::LogicalOr>(const data::Value &left, const data::Value &right)
{    // logical operator '||' OR
    try {
        if (std::get<data::BasicType>(left.type()) == data::BasicType::Boolean &&
            std::get<data::BasicType>(right.type()) == data::BasicType::Boolean)
            return data::Value(std::get<bool>(left) || std::get<bool>(right));    // perform logical OR
        else                                                                      // unsupported
            throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                                   "Invalid Logical '||' operator between these types.");
    } catch (InvalidException e) {
        throw e;
    } catch (std::bad_variant_access e) {
        throw InvalidException("InvalidOperation", (left.type().toString() + "," + right.type().toString()).c_str(),
                               "Invalid Logical '||' operator between these types.");
    }
}

// Logical operators -- end

}    // end of namespace pivot::ecs::script::interpreter::builtins