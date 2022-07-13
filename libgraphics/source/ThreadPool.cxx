#include "pivot/graphics/ThreadPool.hxx"

#include "pivot/pivot.hxx"

#include <Logger.hpp>

namespace pivot
{

void ThreadPool::start(unsigned i)
{
    state.bExit = false;
    resize(i);
}

void ThreadPool::stop()
{
    state.bExit = true;
    state.q_var.notify_all();
    thread_p.clear();
}

void ThreadPool::resize(unsigned size)
{
    unsigned old_size = thread_p.size();
    thread_p.resize(size);
    for (; old_size < thread_p.size(); old_size++) {
        thread_p.at(old_size) = std::jthread(&ThreadPool::new_thread, std::ref(state), old_size);
    }
}

void ThreadPool::new_thread(State &state, unsigned id) noexcept
{
    WorkUnits work;

    logger.trace(function_name()) << "New thread: " << id;
    while (!state.bExit) {
        try {
            {
                std::unique_lock lock(state.q_mutex);
                if (state.qWork.empty()) state.q_var.wait(lock);
                /// lock is owned by this thread when .wait return

                if (state.qWork.empty()) continue;

                work = std::move(state.qWork.front());
                state.qWork.pop();
            }
            if (work) work(id);
        } catch (const std::exception &e) {
            logger.err("Thread Pool") << id << " : " << e.what();
        } catch (...) {
            logger.err("Thread Pool") << "Unkown error on thread " << id;
        }
    }
    logger.trace(function_name()) << "End thread: " << id;
}

}    // namespace pivot
