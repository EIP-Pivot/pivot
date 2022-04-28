#pragma once

#include <stdexcept>

namespace pivot::graphics::internal
{
/// Base class for pivot exceptions
class PivotException : public std::exception
{
public:
    /// return the scope of the exception
    virtual const char *const getScope() const noexcept = 0;
};
}    // namespace pivot::graphics::internal

#define RUNTIME_EXCEPTION(name)                                                                          \
    struct name##Error : public std::runtime_error, public ::pivot::graphics::internal::PivotException { \
        using std::runtime_error::runtime_error;                                                         \
        const char *const getScope() const noexcept override { return #name; }                           \
    };

#define LOGIC_EXCEPTION(name)                                                                          \
    struct name##Error : public std::logic_error, public ::pivot::graphics::internal::PivotException { \
        using std::logic_error::logic_error;                                                           \
        const char *const getScope() const noexcept override { return #name; }                         \
    }
