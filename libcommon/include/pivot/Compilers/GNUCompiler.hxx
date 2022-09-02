#include "pivot/debug.hxx"

#if !defined(COMPILER_GNU)

static_assert(false, "GNU Compiler header included without compiling with GNU");

#endif

namespace pivot
{
class GNUCompiler
{
    FORCEINLINE void *return_address() { return __builtin_return_address(0); }
    FORCEINLINE void *return_address_pointer() { return __builtin_extract_return_addr(return_address()); }
};

using Compiler = GNUCompiler;

}    // namespace pivot
