#pragma once

#include <exception>
#include <string>

namespace pivot::ecs::script
{

class MixedIndentException : public std::runtime_error
{
    size_t linenb;             // line of the indent error
    const char *line;          // raw line of the error
    const char *info;          // info on the error
public:
    MixedIndentException(const char *msg, const char *line_, size_t linenb_, const char *info_)
        : std::runtime_error(msg), line(line_), linenb(linenb_), info(info_)
    {
    }
    size_t get_line_nb() const { return linenb; }
    const char *get_line() const { return line; }
    const char *get_info() const { return info; }
};

class TokenException : public std::runtime_error
{
    const char *token;            // token that produced the error
    size_t linenb;                // line position of the error
    size_t charnb;                // char position on the line
    const char *exctype;          // WARNING (non-blocking) or ERROR (blocking)
    const char *info;             // specification
public:
    TokenException(const char *exceptionType, const char *token_, size_t linenb_, size_t charnb_, const char *exctype_,
                   const char *info_)
        : std::runtime_error(exceptionType),
          token(token_),
          linenb(linenb_),
          charnb(charnb_),
          exctype(exctype_),
          info(info_)
    {
    }
    const char *get_token() const { return token; }
    size_t get_line_nb() const { return linenb; }
    size_t get_char_nb() const { return charnb; }
    const char *get_exctype() const { return exctype; }
    const char *get_info() const { return info; }
};

class InvalidException : public std::runtime_error
{
    const char *exceptionType;    // precision to the type of error
    const char *faulter;          // repr of what caused the error
    const char *error;            // specification
public:
    InvalidException(const char *exceptionType_, const char *faulter_, const char *error_)
        : std::runtime_error(exceptionType_), exceptionType(exceptionType_), faulter(faulter_), error(error_)
    {
    }
    const char *getExceptionType() const { return exceptionType; }
    const char *getFaulter() const { return faulter; }
    const char *getError() const { return error; }
};

}    // end of namespace pivot::ecs::script
