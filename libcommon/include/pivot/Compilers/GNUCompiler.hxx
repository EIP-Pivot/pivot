#include "pivot/debug.hxx"

#if !defined(COMPILER_GNU)

static_assert(false, "GNU Compiler header included without compiling with GNU");

#endif

namespace pivot
{
class GNUCompiler
{
public:
    FORCEINLINE static void *return_address() { return __builtin_return_address(0); }
    FORCEINLINE static void *return_address_pointer() { return __builtin_extract_return_addr(return_address()); }

    [[noreturn]] FORCEINLINE static void unreachable() { __builtin_unreachable(); }
};

using Compiler = GNUCompiler;

}    // namespace pivot
