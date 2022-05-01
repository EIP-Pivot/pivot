#pragma once

#include <exception>
#include <string>

namespace pivot::ecs::script
{

class MixedIndentException : public std::runtime_error
{
public:
    MixedIndentException(const char *msg, const char *line_, size_t linenb_, const char *info_)
        : std::runtime_error(msg), line(line_), linenb(linenb_), info(info_)
    {
    }
    /// return the error line number
    size_t get_line_nb() const { return linenb; }
    /// return the error raw line
    const char *get_line() const { return line; }
    /// return the error specification
    const char *get_info() const { return info; }

private:
    size_t linenb;
    const char *line;
    const char *info;
};

class TokenException : public std::runtime_error
{
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
    /// return the token that caused the error
    const char *get_token() const { return token; }
    /// return the error line number
    size_t get_line_nb() const { return linenb; }
    /// return the error char number
    size_t get_char_nb() const { return charnb; }
    /// return the error type
    const char *get_exctype() const { return exctype; }
    /// return the error specification
    const char *get_info() const { return info; }

private:
    const char *token;
    size_t linenb;
    size_t charnb;
    const char *exctype;
    const char *info;
};

class InvalidException : public std::runtime_error
{
public:
    InvalidException(const char *exceptionType_, const char *faulter_, const char *error_)
        : std::runtime_error(exceptionType_), exceptionType(exceptionType_), faulter(faulter_), error(error_)
    {
    }
    /// return the error type
    const char *getExceptionType() const { return exceptionType; }
    /// return the representation of what caused the error
    const char *getFaulter() const { return faulter; }
    /// return the error specification
    const char *getError() const { return error; }

private:
    const char *exceptionType;
    const char *faulter;
    const char *error;
};

}    // end of namespace pivot::ecs::script
