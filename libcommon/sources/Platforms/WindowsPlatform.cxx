#include "pivot/Platforms/WindowsPlatform.hxx"

#include <debugapi.h>
#include <processthreadsapi.h>

namespace pivot
{

bool WindowsPlatform::isDebuggerPresent() { return !!::IsDebuggerPresent(); }

void WindowsPlatform::setThreadName(std::jthread &thread, const std::string &name)
{
    std::wstring nameStupidType(name.begin(), name.end());
    HRESULT hr = ::SetThreadDescription(thread.native_handle(), nameStupidType.c_str());
    if (FAILED(hr)) { logger.err("WindowsPlatform::setThreadName") << "SetThreadDescription('" << name << "') failed"; }
}

static std::string getThreadName(std::jthread &thread)
{
    PWSTR name;
    HRESULT hr = ::GetThreadDescription(thread.native_handle(), &name);
    if (SUCCEEDED(hr)) {
        std::wstring nameNoStupidType(name, wcslen(name));
        LocalFree(name);
        return std::string(nameNoStupidType.begin(), nameNoStupidType.end());
    }
    return "";
}

}    // namespace pivot
