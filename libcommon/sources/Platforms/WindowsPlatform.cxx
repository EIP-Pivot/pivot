#include "pivot/Platforms/WindowsPlatform.hxx"

#include <debugapi.h>

namespace pivot
{

bool WindowsPlatform::isDebuggerPresent() { return !!::IsDebuggerPresent(); }

}    // namespace pivot
