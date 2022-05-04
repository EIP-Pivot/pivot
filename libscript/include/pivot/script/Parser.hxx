#pragma once

#include "pivot/script/DataStructures.hxx"

#include <queue>

namespace pivot::ecs::script::parser
{

/*! \brief	Parser/Lexer handler to parse files and extract ASTs
 *
 * 	The lexer will analyze files and extract tokens based on symbols.
 * 	It will pass these tokens to the parser which will extract nodes
 * 	and generate an AST based on syntax.
 *
 */
class Parser
{
public:
    ///	Main function, will return the abstract syntax tree of the file (root Node)
    Node ast_from_file(const std::string &file, bool isContent = false, bool verbose = true);
    /// Lexer function, will return a list of Tokens contained in the file
    void tokens_from_file(const std::string &file, bool isContent = false, bool verbose = false);
    /// Strictly for test purposes
    constexpr const std::queue<Token> &getTokenQueue() const { return _tokens; };

private:
    /// Consume a component token and all following to build a component declaration node
    Node consumeComponent();
    /// Consume one token
    void consumeComponentToken(Node &result, TokenType expectedType, NodeType fillType, Token &lastToken);
    /// Consume a system token and all following to build a system declaration node
    Node consumeSystem();
    /// Consume one token for declaration (first of queue)
    void consumeSystemDescriptionToken(Node &result, TokenType expectedType, NodeType fillType, Token &lastToken);
    /// Consume one token for declaration (parameter)
    void consumeSystemDescriptionToken(const Token &token, Node &result, TokenType expectedType, NodeType fillType,
                                       Token &lastToken);
    /// Consume an entire statement and append it to children node
    void consumeSystemStatement(Node &result, Token &lastToken);
    /// Consume entire block and append it to children node
    void consumeSystemBlock(Node &result, Token &lastToken);
    /// Consume a variable and append it to children node
    void consumeSystemVariable(Node &result, Token &lastToken);
    /// Consume an expression and append it to children node
    void consumeSystemExpression(Node &result, Token &lastToken);
    /// Consume function parameters and append them to children node
    void consumeSystemFuncParams(Node &result, Token &lastToken);

    /// Check that token exists, and is of correct type (and potentially consume it from tokens)
    void expectSystemToken(TokenType expectedType, Token &lastToken, bool consume);
    /// Check that token exists, and is of correct value (and potentially consume it from tokens)
    void expectSystemTokenValue(const std::string &expectedValue, Token &lastToken, bool consume);
    /// Check for end of system/component declaration
    bool isDeclarationOver();

public:
private:
    std::queue<Token> _tokens;
};

// Private functions (only called in Parser.cxx but in header for tests)

/// Return whether or not op has higher precedence than compareTo
bool hasHigherPrecedence(const std::string &op, const std::string &compareTo);
/// Number(enum) representing the priority of op
Precedence precedenceOf(const std::string &op);

/// Detect indent size of file content
size_t indent_size_of(const std::string &fileString);
/// Detect indent size of line
size_t indent_size_of_line(const std::string &fileLine);
/// Detect indent type of file content
IndentType indent_type_of(const std::string &fileString);
/// Detect indent type of line
IndentType indent_type_of_line(const std::string &fileLine);
/// Detect indent type of char
IndentType indent_type_of_char(char c);

/// Pretty print a file Node
void printFileNode(const Node &file);
/// Pretty print a component declaration Node
void printComponentNode(const Node &component);
/// Clean input
std::string remove_comments(const std::string &line);
/// Is line empty (or whitespace)
bool line_is_empty(const std::string &line);

}    // end of namespace pivot::ecs::script::parser
