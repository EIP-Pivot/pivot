#include "pivot/debug.hxx"

#if !defined(COMPILER_CLANG)

static_assert(false, "Clang Compiler header included without compiling with clang");

#endif

namespace pivot
{
///
/// @brief Wrapper arround clang intrinsics function
///
class ClangCompiler
{
public:
    /// Return the address of the current function,
    FORCEINLINE static void *return_address() { return __builtin_return_address(0); }
    /// Returns the address of the function frame
    FORCEINLINE static void *return_address_pointer() { return __builtin_frame_address(0); }

    /// Mark a branch as unreachable
    [[noreturn]] FORCEINLINE static void unreachable() { __builtin_unreachable(); }
};

/// Alias of the correct compiler currently used
using Compiler = ClangCompiler;

}    // namespace pivot
