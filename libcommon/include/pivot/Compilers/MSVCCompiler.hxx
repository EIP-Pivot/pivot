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
    FORCEINLINE void *return_address() { return _ReturnAddress(); }
    FORCEINLINE void *return_address_pointer() { return _AddressOfReturnAddress(); }
};

using Compiler = MSVCCompiler;

}    // namespace pivot
