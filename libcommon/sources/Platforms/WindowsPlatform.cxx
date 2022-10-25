#include "pivot/Platforms/WindowsPlatform.hxx"

#include <debugapi.h>
#include <processthreadsapi.h>

namespace pivot
{

bool WindowsPlatform::isDebuggerPresent() { return !!::IsDebuggerPresent(); }

void WindowsPlatform::setThreadName(std::jthread &thread, const std::string &name)
{
    HRESULT hr = ::SetThreadDescription(thread.native_handle(), std::wstring(name.begin(), name.end()));
    if (FAILED(hr)) { logger.err("WindowsPlatform::setThreadName") << "SetThreadDescription('" << name << "') failed"; }
}

static std::string getThreadName(std::jthread &thread)
{
    PWSTR *name = nullptr;
    HRESULT hr = ::GetThreadDescription(thread.native_handle(), &name);
    if (SUCCEEDED(hr)) {
        std::string nameNoStupidType(name);
        LocalFree(name);
        return nameNoStupidType;
    }
    return "";
}

}    // namespace pivot
