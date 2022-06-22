#include "pivot/graphics/ThreadPool.hxx"

#include "pivot/graphics/pivot.hxx"

#include <Logger.hpp>

namespace pivot
{

void ThreadPool::start(unsigned i)
{
    bExit = false;
    this->resize(i);
}

void ThreadPool::stop()
{
    bExit = true;
    q_var.notify_all();
    thread_p.clear();
}

void ThreadPool::resize(unsigned size)
{
    unsigned old_size = thread_p.size();
    thread_p.resize(size);
    for (; old_size < thread_p.size(); old_size++) {
        this->thread_p.at(old_size) = std::jthread(&ThreadPool::new_thread, this, old_size);
    }
}

void ThreadPool::new_thread(unsigned id) noexcept
{
    WorkUnits work;

    logger.trace(file_position()) << "New thread: " << id;
    while (!bExit) {
        try {
            {
                std::unique_lock lock(q_mutex);
                if (qWork.empty()) q_var.wait(lock);
                /// lock is owned by this thread when .wait return

                if (qWork.empty()) continue;

                work = std::move(this->qWork.front());
                this->qWork.pop();
            }
            if (work) work(id);
        } catch (const std::exception &e) {
            logger.err("Thread Pool") << id << " : " << e.what();
        } catch (...) {
            logger.err("Thread Pool") << "Unkown error on thread " << id;
        }
    }
    logger.trace(file_position()) << "End thread: " << id;
}

}    // namespace pivot
