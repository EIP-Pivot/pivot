#include "pivot/debug.hxx"

#if !defined(COMPILER_CLANG)

static_assert(false, "Clang Compiler header included without compiling with clang");

#endif

namespace pivot
{
class ClangCompiler
{
    FORCEINLINE void *return_address() { return __builtin_return_address(0); }
    FORCEINLINE void *return_address_pointer() { return __builtin_frame_address(0); }
};

using Compiler = ClangCompiler;

}    // namespace pivot
