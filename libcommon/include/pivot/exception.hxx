#pragma once

#include <stdexcept>
#include <string_view>

namespace pivot
{
/// Base class for pivot exceptions
class PivotException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
    /// return the scope of the exception
    virtual std::string getScope() const noexcept = 0;
    /// return the kind of the exception
    virtual std::string getKind() const noexcept = 0;
};

/// Pivot Logic Error
class LogicError : public PivotException
{
    using PivotException::PivotException;
    std::string getKind() const noexcept final { return "Logic"; };
};

/// Pivot Runtime Error
class RuntimeError : public PivotException
{
    using PivotException::PivotException;
    std::string getKind() const noexcept final { return "Runtime"; };
};

}    // namespace pivot

#define PIVOT_ERROR(kind, name)                                          \
    struct name##Error : public ::pivot::kind {                          \
        using kind::kind;                                                \
        std::string getScope() const noexcept override { return #name; } \
    };                                                                   \
    static_assert(std::is_nothrow_copy_constructible_v<name##Error>);    \
    static_assert(std::is_nothrow_copy_assignable_v<name##Error>);

#define RUNTIME_ERROR(name) PIVOT_ERROR(RuntimeError, name)

#define LOGIC_ERROR(name) PIVOT_ERROR(LogicError, name)
