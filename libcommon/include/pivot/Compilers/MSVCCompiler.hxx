#pragma once

#include "pivot/debug.hxx"

#if !defined(COMPILER_MSVC)

static_assert(false, "MSVC Compiler header included without compiling with MSVC");

#endif

#include <intrin.h>

namespace pivot
{

///
/// @brief Wrapper around MSCV intrisics functions
///
class MSVCCompiler
{
public:
    /// Return the address of the current function,
    FORCEINLINE static void *return_address() { return _ReturnAddress(); }
    /// Returns the address of the function frame
    FORCEINLINE static void *return_address_pointer() { return _AddressOfReturnAddress(); }

    /// Mark a branch as unreachable
    [[noreturn]] FORCEINLINE static void unreachable() { __assume(false); }
};

/// Alias of the correct compiler currently used
using Compiler = MSVCCompiler;

}    // namespace pivot
