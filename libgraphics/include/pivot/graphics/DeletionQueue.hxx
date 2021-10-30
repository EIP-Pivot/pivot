#pragma once

#include <deque>
#include <functional>

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
    template <typename F>
    requires std::is_convertible_v<F &&, std::function<void()>>
    inline void push(F &&function)
    {
        static_assert(sizeof(F) < 200, "Don't capture too much in the lambda !");
        deletor.push_back(function);
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
