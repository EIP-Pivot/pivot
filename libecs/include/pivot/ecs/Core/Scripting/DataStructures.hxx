#pragma once

#include <string>
#include <vector>
#include <format> // TODO : remove, not in C++20 gcc yet
#include <exception>

#include "pivot/ecs/Core/Data/value.hxx"

namespace pivot::ecs::script {

enum class IndentType {
	Tabs,
	Spaces,
	NoIndent,
	Invalid
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
	FunctionParams,
	Operator,
	Type,
	Name,
	Symbol
};

enum class TokenType {
	Identifier,
	LiteralNumber,
	Symbol,
	Indent,
	Dedent
};

struct Node {
	NodeType type;
	std::string value;
	size_t line_nb;
	size_t char_nb;

	std::vector<Node> children;
};

struct Token {
private:
	inline auto tied() const { return std::tie(type, value, line_nb, char_nb); }
public:
	TokenType type;
	std::string value;
	size_t line_nb;
	size_t char_nb;

	inline std::string toString() const { return std::format("value='{}' line {} char {}", value, line_nb, char_nb); }
	inline bool operator==(Token rhs) const { return tied() == rhs.tied(); }
};


class MixedIndentException : public std::exception {
	size_t linenb;
	const std::string line;
	const std::string info;
	public:
		MixedIndentException(const char* msg, const std::string &line_,  size_t linenb_, const std::string &info_) : std::exception(msg),
			line (line_), linenb (linenb_), info (info_) {}
		size_t get_line_nb() const { return linenb; }
		const std::string &get_line() const { return line; }
		const std::string &get_info() const { return info; }
};

// exceptionType :		WARNING	or	ERROR
// 			WARNING does not interrupt parsing/interpreting but ERROR does
// linenb charnb :		Position of where the error occured
// info	:				Specification to the error

class TokenException : public std::exception {
	const std::string token;
	size_t linenb;
	size_t charnb;
	const std::string exctype;
	const std::string info;
	public:
		TokenException(const char* exceptionType, const std::string &token_, size_t linenb_, size_t charnb_, const std::string &exctype_, const std::string &info_) : std::exception(exceptionType),
			token(token_), linenb (linenb_), charnb(charnb_), exctype(exctype_), info (info_) {}
		const std::string &get_token() const { return token; }
		size_t get_line_nb() const { return linenb; }
		size_t get_char_nb() const { return charnb; }
		const std::string &get_exctype() const { return exctype; }
		const std::string &get_info() const { return info; }
};

} // end of namespace pivot::ecs::script

// bool operator==(const pivot::ecs::script::Token &lhs, const pivot::ecs::script::Token &rhs) const { return lhs.tied() == rhs.tied(); }
