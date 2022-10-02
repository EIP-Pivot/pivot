#pragma once

#include <chrono>
#include <ratio>
#include <thread>

#include <pivot/pivot.hxx>

template <unsigned FPS>
class FrameLimiter
{
public:
    FrameLimiter(): time_frame(1), tp(std::chrono::steady_clock::now()){};

    void sleep()
    {
        DEBUG_FUNCTION;
        tp += time_frame;
        std::this_thread::sleep_until(tp);
    };

private:
    std::chrono::duration<double, std::ratio<1, FPS>> time_frame;
    std::chrono::time_point<std::chrono::steady_clock, decltype(time_frame)> tp;
};
