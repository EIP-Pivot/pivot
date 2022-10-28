#pragma once

#include <memory>
#include <string>
#include <thread>

#include "pivot/interface/ThreadRuntime.hxx"

namespace pivot
{

class Thread
{
public:
    Thread();
    Thread(Thread &other) = delete;
    Thread(Thread &&other) = default;
    virtual ~Thread();
    void start();
    void end(bool bShouldWait = true);

    void create(const std::string &name, std::unique_ptr<internal::IThreadRuntime> threadCode);

protected:
    virtual void preRun();
    virtual std::uint32_t run();
    virtual void postRun();

private:
    static void thread_runtime(Thread *pThis);

private:
    std::string m_name;
    std::jthread m_managedThread;
    std::unique_ptr<internal::IThreadRuntime> m_internalRuntime;
};

}    // namespace pivot