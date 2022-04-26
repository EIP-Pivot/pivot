#pragma once

#include <exception>
#include <string>

namespace pivot::ecs::script {

class MixedIndentException : public std::exception {
	size_t linenb; // line of the indent error
	const std::string line; // raw line of the error
	const std::string info; // info on the error
	public:
		MixedIndentException(const char* msg, const std::string &line_,  size_t linenb_, const std::string &info_) : std::exception(msg),
			line (line_), linenb (linenb_), info (info_) {}
		size_t get_line_nb() const { return linenb; }
		const std::string &get_line() const { return line; }
		const std::string &get_info() const { return info; }
};

class TokenException : public std::exception {
	const std::string token; // token that produced the error
	size_t linenb; // line position of the error
	size_t charnb; // char position on the line
	const std::string exctype; // WARNING (non-blocking) or ERROR (blocking)
	const std::string info; // specification
	public:
		TokenException(const char* exceptionType, const std::string &token_, size_t linenb_, size_t charnb_, const std::string &exctype_, const std::string &info_) : std::exception(exceptionType),
			token(token_), linenb (linenb_), charnb(charnb_), exctype(exctype_), info (info_) {}
		const std::string &get_token() const { return token; }
		size_t get_line_nb() const { return linenb; }
		size_t get_char_nb() const { return charnb; }
		const std::string &get_exctype() const { return exctype; }
		const std::string &get_info() const { return info; }
};

class InvalidException : public std::exception {
	const std::string exceptionType; // precision to the type of error
	const std::string faulter; // repr of what caused the error
	const std::string error; // specification
	public:
		InvalidException(const char *exceptionType_, const std::string &faulter_, const std::string &error_): std::exception(exceptionType_),
			exceptionType(exceptionType_), faulter(faulter_), error(error_) {}
		const std::string &getExceptionType() const { return exceptionType; }
		const std::string &getFaulter() const { return faulter; }
		const std::string &getError() const { return error; }
};

} // end of namespace pivot::ecs::script
