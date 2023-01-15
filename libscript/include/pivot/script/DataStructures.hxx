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
    EventDeclaration,
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
    EventEntityParameter,
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
    Boolean,
    FunctionCall,
    FunctionParams,
    Operator,
    Type,
    Name,
    Symbol,
    EmitEvent,
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
    Boolean,
    Symbol,
    Indent,
    DoubleQuotedString,
    Dedent,
};

/// An expression op is either an operator or an operand
/// A chain of these represents an entire expression (stored in postfix)
struct ExpressionOp {
    /// Whether this is an operator or an operand
    bool isOperator;
    /// Value of the operand
    data::Value operand;
    /// String representing the operator
    std::string operatorStr;
};

/// Used by the parser to create Abstract Syntax Trees (AST)
/// A file is represented as a root node containing all file statements
struct Node {
    /// Type of the node
    NodeType type;
    /// String value of the node
    std::string value;
    /// Starting line number of the node
    size_t line_nb;
    /// Starting char number of the node
    size_t char_nb;

    /// Children nodes
    std::vector<Node> children;
};

/// A token is a string of characters representing 'something' in .pvt
/// A file is parsed into tokens by the lexer, which are analyzed by the parser
/// To generate an AST
struct Token {
public:
    // inline std::string toString() const { return std::format("value='{}' line {} char {}", value, line_nb, char_nb);
    // } // format not available in c++20 gcc yet
    /// Return string representing the token
    inline std::string toString() const
    {
        return "value='" + value + "' line " + std::to_string(line_nb) + " char " + std::to_string(line_nb);
    }
    /// Starship operator for comparing tokens
    auto operator<=>(const Token &rhs) const = default;

public:
    /// Type of the token
    TokenType type;
    /// String value of the token
    std::string value;
    /// Starting line number of the token
    size_t line_nb;
    /// Starting char number of the token
    size_t char_nb;
};

}    // end of namespace pivot::ecs::script