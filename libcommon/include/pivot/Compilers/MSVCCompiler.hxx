#pragma once

#include "pivot/debug.hxx"

#if !defined(COMPILER_MSVC)

static_assert(false, "MSVC Compiler header included without compiling with MSVC");

#endif

#include <intrin.h>

namespace pivot
{
class MSVCCompiler
{
public:
    FORCEINLINE static void *return_address() { return _ReturnAddress(); }
    FORCEINLINE static void *return_address_pointer() { return _AddressOfReturnAddress(); }

    [[noreturn]] FORCEINLINE static void unreachable() { __assume(false); }
};

using Compiler = MSVCCompiler;

}    // namespace pivot
