#pragma once

#include "pivot/script/DataStructures.hxx"

#include <stack>

namespace pivot::ecs::script::parser {

//	Main function, will return the abstract syntax tree of the file (root Node)
Node ast_from_file(const std::string &filename, bool verbose = true);


// Lexer function, will return a list of Tokens contained in the file
std::vector<Token> tokens_from_file(const std::string &filename, bool verbose = false);



// private functions
Node consumeComponent(std::vector<Token> &tokens); // Consume a component token and all following to build a component declaration node
void consumeComponentToken(std::vector<Token> &tokens, Node &result, TokenType expectedType, NodeType fillType, Token &lastToken); // consume one token
Node consumeSystem(std::vector<Token> &tokens); // Consume a system token and all following to build a system declaration node
void consumeSystemDescriptionToken(std::vector<Token> &tokens, Node &result, TokenType expectedType, NodeType fillType, Token &lastToken); // consume one token for declaration
void consumeSystemStatement(std::vector<Token> &tokens, Node &result, Token &lastToken); // consume an entire statement and append it to children node
void consumeSystemBlock(std::vector<Token> &tokens, Node &result, Token &lastToken); // consume entire block and append it to children node
void consumeSystemVariable(std::vector<Token> &tokens, Node &result, Token &lastToken); // consume a variable and append it to children node
void consumeSystemExpression(std::vector<Token> &tokens, Node &result, Token &lastToken); // consume an expression and append it to children node
void consumeSystemFuncParams(std::vector<Token> &tokens, Node &result, Token &lastToken); // consume function parameters and append them to children node

void expectSystemToken(std::vector<Token> &tokens, TokenType expectedType, Token &lastToken, bool consume); // check that token exists, and is of correct type (and potentially consume it from tokens)
void expectSystemTokenValue(std::vector<Token> &tokens, const std::string &expectedValue, Token &lastToken, bool consume); // check that token exists, and is of correct value (and potentially consume it from tokens)
bool isDeclarationOver(const std::vector<Token> &tokens); // Check for end of system/component declaration

// Detect indent size of file content
size_t indent_size_of(const std::string &fileString);
size_t indent_size_of_line(const std::string &fileLine);
// Detect indent type of file content
IndentType indent_type_of(const std::string &fileString);
IndentType indent_type_of_line(const std::string &fileLine);
IndentType indent_type_of_char(char c);


void printFileNode(const Node &file); // Pretty print a file Node
void printComponentNode(const Node &component); // Pretty print a component declaration Node
std::string remove_comments(const std::string &line); // Clean input
bool line_is_empty(const std::string &line); // Is line empty (or whitespace)

} // end of namespace pivot::ecs::script::parser
