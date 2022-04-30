#pragma once

#include <stdexcept>

namespace pivot
{
/// Base class for pivot exceptions
class PivotException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
    /// return the scope of the exception
    virtual const std::string_view getScope() const noexcept = 0;
    /// return the kind of the exception
    virtual const std::string_view getKind() const noexcept = 0;
};

/// Pivot Logic Error
class LogicError : public PivotException
{
    using PivotException::PivotException;
    const std::string_view getKind() const noexcept final { return "Logic"; };
};

/// Pivot Runtime Error
class RuntimeError : public PivotException
{
    using PivotException::PivotException;
    const std::string_view getKind() const noexcept final { return "Logic"; };
};

}    // namespace pivot

#define PIVOT_EXCEPTION(kind, name)                                                 \
    struct name##Error : public ::pivot::kind {                                     \
        using kind::kind;                                                           \
        const std::string_view getScope() const noexcept override { return #name; } \
    };                                                                              \
    static_assert(std::is_nothrow_copy_constructible_v<name##Error>);               \
    static_assert(std::is_nothrow_copy_assignable_v<name##Error>);

#define RUNTIME_EXCEPTION(name) PIVOT_EXCEPTION(RuntimeError, name)

#define LOGIC_EXCEPTION(name) PIVOT_EXCEPTION(LogicError, name)
