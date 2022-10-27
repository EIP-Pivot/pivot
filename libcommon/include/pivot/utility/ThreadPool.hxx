#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <type_traits>

#include "pivot/interface/ThreadRuntime.hxx"
#include "pivot/pivot.hxx"
#include "pivot/utility/Thread.hxx"

namespace pivot
{

/// Manage a set of thread for sheduling work
class ThreadPool
{
public:
    using WorkUnits = std::function<void(unsigned id)>;

    struct State {
        std::mutex q_mutex;
        std::condition_variable q_var;
        std::queue<WorkUnits> qWork;
    };

public:
    PIVOT_NO_COPY_NO_MOVE(ThreadPool)
    ThreadPool();
    ~ThreadPool() = default;

    /// Create the pool with a given number of thread
    void start(unsigned size = std::max(((std::thread::hardware_concurrency() * 2) / 3), 1u));
    /// Stop and join all of the thread
    void stop();
    /// Return the amount of thread in the pool
    size_t size() { return thread_p.size(); }
    /// Resize the thread pool
    void resize(unsigned size);

    template <class F, typename... Args>
    /// Push a new job in the pool and return a future
    requires std::is_invocable_v<F, unsigned, Args...>
    [[nodiscard]] auto push(F &&f, Args &&...args) -> std::future<decltype(f(0, args...))>
    {
        DEBUG_FUNCTION();
        if (!verifyAlwaysMsg(!thread_p.empty(), "Pushing task when no thread are started !")) { start(1); }
        auto packagedFunction = std::make_shared<std::packaged_task<decltype(f(0, args...))(unsigned)>>(
            std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...));

        WorkUnits storageFunc([packagedFunction](int id) { (*packagedFunction)(id); });

        {
            std::unique_lock lock(state->q_mutex);
            state->qWork.push(storageFunc);
        }
        state->q_var.notify_one();
        return packagedFunction->get_future();
    }

private:
    std::shared_ptr<State> state;
    std::vector<Thread> thread_p;
};

class WorkerPoolRuntime : public internal::IThreadRuntime
{
public:
    WorkerPoolRuntime(std::shared_ptr<ThreadPool::State> context);

    bool init() override;
    std::uint32_t run() override;
    void stop() override;
    void exit() override;

private:
    static std::atomic_int i_threadIDCounter;

private:
    int i_threadID;
    std::atomic_bool b_requestExit;
    std::shared_ptr<ThreadPool::State> p_state;
};

}    // namespace pivot
