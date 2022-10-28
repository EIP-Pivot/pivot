#include "pivot/Threading/Thread.hxx"
#include "pivot/pivot.hxx"

#include <mutex>

namespace pivot
{

Thread::Thread(): m_name(), m_internalRuntime(nullptr) {}
Thread::~Thread() { end(); }

void Thread::start() {}

void Thread::end(bool bShouldWait)
{
    if (m_internalRuntime) { m_internalRuntime->stop(); }
    m_managedThread.request_stop();

    if (bShouldWait && m_managedThread.joinable()) { m_managedThread.join(); }
}

void Thread::create(const std::string &name, std::unique_ptr<internal::IThreadRuntime> threadCode)
{
    if (m_managedThread.joinable()) {
        m_managedThread.request_stop();
        m_managedThread.join();
    }

    m_internalRuntime = std::move(threadCode);
    m_name = name;

    m_managedThread = std::jthread(thread_runtime, this);
    Platform::setThreadName(m_managedThread, m_name);
}

void Thread::preRun() {}

std::uint32_t Thread::run()
{
    std::uint32_t exitCode = 1;
    pivotAssert(m_internalRuntime);

    if (m_internalRuntime->init()) {
        preRun();
        exitCode = m_internalRuntime->run();
        m_internalRuntime->exit();
    }

    logger.info("Thread") << "Thread (\"" << m_name << "\") exited with code " << exitCode;
    return exitCode;
}

void Thread::postRun() {}

void Thread::thread_runtime(Thread *pThis)
{
    pivotAssert(pThis);
    pThis->preRun();
    pThis->run();
    pThis->postRun();
}

}    // namespace pivot