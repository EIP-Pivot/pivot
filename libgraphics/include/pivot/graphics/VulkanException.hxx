#pragma once

#include <stdexcept>

namespace pivot::graphics::internal
{
/// Base class for pivot exceptions
class PivotException : public std::exception
{
public:
    PivotException() = delete;
    /// main ctor
    PivotException(const std::string &msg): msg(msg) {}
    /// return the scope of the exception
    virtual const char *const getScope() const noexcept = 0;
    /// return the kind of the exception
    virtual const char *const getKind() const noexcept = 0;
    /// return both of the above

    /// return the error message
    const char *what() const noexcept override { return msg.c_str(); }

private:
    const std::string msg;
};

class PivotLogicError : public PivotException
{
    using PivotException::PivotException;
    const char *const getKind() const noexcept final { return "Logic"; };
};

class PivotRuntimeError : public PivotException
{
    using PivotException::PivotException;
    const char *const getKind() const noexcept final { return "Logic"; };
};

}    // namespace pivot::graphics::internal

#define PIVOT_EXCEPTION(kind, name)                                            \
    struct name##Error : public ::pivot::graphics::internal::kind {            \
        using kind::kind;                                                      \
        const char *const getScope() const noexcept override { return #name; } \
    };

#define RUNTIME_EXCEPTION(name) PIVOT_EXCEPTION(PivotRuntimeError, name)

#define LOGIC_EXCEPTION(name) PIVOT_EXCEPTION(PivotLogicError, name)
