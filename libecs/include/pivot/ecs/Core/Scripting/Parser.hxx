#pragma once

#include "pivot/ecs/Core/Scripting/DataStructures.hxx"

#include <stack>

namespace pivot::ecs::script::parser {

//	Main function, will return the abstract syntax tree of the file (root Node)
Node ast_from_file(const std::string &filename, bool verbose = true);


// Lexer function, will return a list of Tokens contained in the file
std::vector<Token> tokens_from_file(const std::string &filename, bool verbose = false);



// private functions
Node consumeComponent(std::vector<Token> &tokens);
void consumeComponentToken(std::vector<Token> &tokens, Node &result, TokenType expectedType, NodeType fillType, Token &lastToken);
Node consumeSystem(std::vector<Token> &tokens);
void consumeSystemDescriptionToken(std::vector<Token> &tokens, Node &result, TokenType expectedType, NodeType fillType, Token &lastToken);
void consumeSystemStatement(std::vector<Token> &tokens, Node &result, Token &lastToken);
void consumeSystemBlock(std::vector<Token> &tokens, Node &result, Token &lastToken);
void consumeSystemVariable(std::vector<Token> &tokens, Node &result, Token &lastToken);
void consumeSystemOperator(std::vector<Token> &tokens, Node &result, Token &lastToken);
void consumeSystemExpression(std::vector<Token> &tokens, Node &result, Token &lastToken);
void consumeSystemFuncParams(std::vector<Token> &tokens, Node &result, Token &lastToken);
void expectSystemToken(std::vector<Token> &tokens, TokenType expectedType, Token &lastToken, bool consume);
void expectSystemTokenValue(std::vector<Token> &tokens, const std::string &expectedValue, Token &lastToken, bool consume);

// Check for end of system/component declaration
bool isDeclarationOver(const std::vector<Token> &tokens);
// Detect indent size of file content
size_t indent_size_of(const std::string &fileString);
size_t indent_size_of_line(const std::string &fileLine);
// Detect indent type of file content
IndentType indent_type_of(const std::string &fileString);
IndentType indent_type_of_line(const std::string &fileLine);
IndentType indent_type_of_char(char c);

// Pretty print a file Node
void printFileNode(const Node &file);
// Pretty print a component declaration Node
void printComponentNode(const Node &component);
// Clean input
std::string remove_comments(const std::string &line);
// Is line empty
bool line_is_empty(const std::string &line);

} // end of namespace pivot::ecs::script::parser
