#pragma once

#include <chrono>
#include <fstream>
#include <string>

#include <pivot/pivot.hxx>
#include <pivot/utility/source_location.hxx>

namespace pivot::benchmark
{

struct TimerResult {
    const std::string name;
    const long long start_timestamp;
    const long long end_timestamp;
    const std::thread::id threadId;
};

class Timer;

class Instrumentor
{
public:
    static Instrumentor &get()
    {
        static Instrumentor instance;
        return instance;
    }

public:
    Instrumentor();
    ~Instrumentor();

    void beginSession(const std::string &filepath = "result.json");
    void endSession();
    void writeResult(TimerResult result);

private:
    void writeHeader();
    void writeFooter();

private:
    std::mutex mutex;
    std::ofstream outputStream;
    int profileCount = 0;
};

class Timer
{
public:
    FORCEINLINE Timer(const std::string name): name(std::move(name))
    {
        startTimepoint = std::chrono::high_resolution_clock::now();
    }
    FORCEINLINE ~Timer()
    {
        if (!stopped) stop();
    }

    FORCEINLINE void stop()
    {
        if (!verify(!stopped)) return;

        auto endTimepoint = std::chrono::high_resolution_clock::now();

        Instrumentor::get().writeResult({
            .name = std::move(name),
            .start_timestamp =
                std::chrono::time_point_cast<std::chrono::microseconds>(startTimepoint).time_since_epoch().count(),
            .end_timestamp =
                std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count(),
            .threadId = std::this_thread::get_id(),
        });

        stopped = true;
    }

private:
    const std::string name;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTimepoint;
    bool stopped = false;
};

}    // namespace pivot::benchmark

#if !defined(NO_BENCHMARK)

    #define PROFILE_SCOPE(name) ::pivot::benchmark::Timer timer##__LINE__(name)
    #define PROFILE_FUNCTION() PROFILE_SCOPE(::pivot::utils::function_name())

#else

    #define PROFILE_SCOPE(name)
    #define PROFILE_FUNCTION()

#endif
