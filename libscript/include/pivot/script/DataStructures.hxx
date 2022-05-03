#pragma once

#include <string>
#include <vector>
// #include <format> // format not available in c++20 gcc yet

#include "pivot/ecs/Core/Data/value.hxx"

namespace pivot::ecs::script
{

enum class IndentType {
    Tabs,
    Spaces,
    NoIndent,
    Invalid,
};

enum class NodeType {
    File,
    ComponentDeclaration,
    SystemDeclaration,
    ComponentName,
    SystemName,
    PropertyType,
    PropertyName,
    EventKeyword,
    EventName,
    EventPayloadType,
    EventPayloadName,
    EventEntityComponent,
    EventEntityName,
    EntityParameterName,
    EntityParameterComponent,
    Indent,
    Dedent,
    SystemEntryPoint,
    Statement,
    Expression,
    NewVariable,
    ExistingVariable,
    LiteralNumberVariable,
    DoubleQuotedStringVariable,
    FunctionParams,
    Operator,
    Type,
    Name,
    Symbol,
};

enum class Precedence {
    LogicalAnd,
    LogicalOr,
    Equality,
    Relational,
    Additive,
    Multiplicative,
    Parens,
};

enum class TokenType {
    Identifier,
    LiteralNumber,
    Symbol,
    Indent,
    DoubleQuotedString,
    Dedent,
};

struct ExpressionOp {
    bool isOperator;
    data::Value operand;
    std::string operatorStr;
};

struct Node {
    NodeType type;
    std::string value;
    size_t line_nb;
    size_t char_nb;

    std::vector<Node> children;
};

struct Token {
public:
    // inline std::string toString() const { return std::format("value='{}' line {} char {}", value, line_nb, char_nb);
    // } // format not available in c++20 gcc yet
    inline std::string toString() const
    {
        return "value='" + value + "' line " + std::to_string(line_nb) + " char " + std::to_string(line_nb);
    }
    auto operator<=>(const Token &rhs) const = default;

public:
    TokenType type;
    std::string value;
    size_t line_nb;
    size_t char_nb;
};

}    // end of namespace pivot::ecs::script