#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <type_traits>

#include <Logger.hpp>

namespace pivot
{

/// Manage a set of thread for sheduling work
class ThreadPool
{
private:
    using WorkUnits = std::function<void(unsigned id)>;

public:
    ThreadPool() = default;
    ThreadPool(const ThreadPool &) = delete;
    ~ThreadPool() = default;

    /// Create the pool with a given number of thread
    void start(unsigned size = std::max(((std::thread::hardware_concurrency() * 2) / 3), unsigned(1)));
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
        if (thread_p.empty()) logger.warn("Thread Pool/push") << "Pushing task when no thread are started !";
        auto packagedFunction = std::make_shared<std::packaged_task<decltype(f(0, args...))(unsigned)>>(
            std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...));

        WorkUnits storageFunc([packagedFunction](int id) { (*packagedFunction)(id); });

        qWork.push(storageFunc);
        q_var.notify_one();
        return packagedFunction->get_future();
    }

private:
    void new_thread(const unsigned i) noexcept;

private:
    std::mutex q_mutex;
    std::condition_variable q_var;
    std::queue<WorkUnits> qWork;

    std::atomic_bool bExit = false;
    std::vector<std::jthread> thread_p;
};

}    // namespace pivot
