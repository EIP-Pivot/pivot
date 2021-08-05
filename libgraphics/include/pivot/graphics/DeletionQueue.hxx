#pragma once

#include <deque>
#include <functional>

class DeletionQueue {
public:
    inline void push(std::function<void()> &&function) { deletor.push_back(function); }

    void flush()
    {
        for (auto it = deletor.rbegin(); it != deletor.rend(); it++) { (*it)(); }
        deletor.clear();
    }

private:
    std::deque<std::function<void()>> deletor;
};
