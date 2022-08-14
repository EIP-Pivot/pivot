#pragma once

#include <deque>
#include <functional>

namespace pivot::graphics
{
/// @class DeletionQueue
///
/// @brief A simple LI.LO struct
///
/// It is used to make Vulkan object lifetime and destruction order easier
class DeletionQueue
{
public:
    /// Push new function in the DeletionQueue
    /// @param function The function to be pushed into the queue

    inline void push(const std::invocable auto function)
    {
        static_assert(sizeof(function) < 200, "Don't capture too much in the lambda !");
        deletor.push_back([function] { function(); });
    }

    /// Flush the queue and execute all the function in reverse order
    ///
    /// @code
    /// DeletionQueue queue;
    ///
    /// queue.push([] { // A });
    /// queue.push([] { // B });
    /// queue.push([] { // C }):
    ///
    /// queue.flush() // will execute C, then B, then A
    /// @endcode
    void flush()
    {
        for (auto it = deletor.rbegin(); it != deletor.rend(); it++) { (*it)(); }
        deletor.clear();
    }

private:
    std::deque<std::function<void()>> deletor;
};
}    // namespace pivot::graphics
