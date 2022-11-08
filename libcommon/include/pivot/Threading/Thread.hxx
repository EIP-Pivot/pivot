#pragma once

#include <memory>
#include <string>
#include <thread>

#include "pivot/interface/ThreadRuntime.hxx"

namespace pivot
{

///
/// @brief Wrapper of the std::jthread
///
///
class Thread
{
public:
    ///
    /// @brief Construct a new Thread object
    ///
    Thread();
    Thread(Thread &other) = delete;
    /// Default move ctor
    Thread(Thread &&other) = default;
    virtual ~Thread();
    /// Initialize the thread
    void start();
    ///
    /// @brief the internal thread should exit
    ///
    /// @param bShouldWait Does the thread should exit now or shutdown gracefully ?
    void end(bool bShouldWait = true);

    ///
    /// @brief Create the actual thread object
    ///
    /// @param name the name of the new thread
    /// @param threadCode The runtime to be executed in the thread
    void create(const std::string &name, std::unique_ptr<internal::IThreadRuntime> threadCode);

protected:
    /// Executed from the thread when started
    virtual void preRun();
    /// Run the internal::IThreadRuntime
    virtual std::uint32_t run();
    /// Executed from the thread when quitting
    virtual void postRun();

private:
    static void thread_runtime(Thread *pThis);

private:
    std::string m_name;
    std::jthread m_managedThread;
    std::unique_ptr<internal::IThreadRuntime> m_internalRuntime;
};

}    // namespace pivot