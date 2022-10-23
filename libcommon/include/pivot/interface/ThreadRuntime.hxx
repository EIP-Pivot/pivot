#pragma once

#include <cstdint>

namespace pivot::internal
{

class IThreadRuntime
{
public:
    virtual ~IThreadRuntime() {}
    virtual bool init() = 0;
    virtual std::uint32_t run() = 0;
    virtual void stop() = 0;
    virtual void exit() = 0;
};

}    // namespace pivot::internal