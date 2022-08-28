#include "pivot/Platform.hxx"

#ifdef PLATFORM_WINDOWS

    #include <debugapi.h>

namespace pivot
{

bool plateform::isDebuggerPresent() { return !!::IsDebuggerPresent(); }

}    // namespace pivot

#endif
