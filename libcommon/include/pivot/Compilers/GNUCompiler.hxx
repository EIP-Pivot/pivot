#include "pivot/debug.hxx"

#if !defined(COMPILER_GNU)

static_assert(false, "GNU Compiler header included without compiling with GNU");

#endif

namespace pivot
{

///
/// @brief Wrapper around GCC intrisics functions
///
class GNUCompiler
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
using Compiler = GNUCompiler;

}    // namespace pivot
