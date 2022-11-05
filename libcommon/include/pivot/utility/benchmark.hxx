#pragma once

#include <chrono>
#include <fstream>
#include <string>

#include "pivot/pivot.hxx"
#include "pivot/utility/define.hxx"
#include "pivot/utility/source_location.hxx"

namespace pivot::benchmark
{

/// Hold the result of a Timer
struct TimerResult {
    /// The name of the section
    const std::string name;
    /// The start point of the timestamp
    const unsigned long long start_timestamp;
    /// The end point of the timestamp
    const unsigned long long end_timestamp;
    /// The id of the thread the section was executed on
    const std::thread::id threadId;
};

class Timer;

///
/// @brief Manage the profiling session and timer
///
class Instrumentor
{
    PIVOT_NO_COPY_NO_MOVE(Instrumentor)
public:
    /// return the global instrumentor
    static Instrumentor &get()
    {
        static Instrumentor instance;
        return instance;
    }

public:
    ///
    /// @brief Construct a new Instrumentor object
    ///
    Instrumentor();
    ///
    /// @brief Destroy the Instrumentor object
    ///
    ~Instrumentor();

    ///
    /// @brief Begin the recording session
    ///
    /// @param filepath the filename where the data will be serialized
    void beginSession(const std::string &filepath = "result.json");
    ///
    /// @brief End the current session
    ///
    void endSession();
    /// Is a session open ?
    bool isSessionStarted() const;
    /// Dump the timer result data in the session file
    void writeResult(const TimerResult &result);

    /// Return the thread name associated with this id
    std::string getThreadName(const std::thread::id &id = std::this_thread::get_id()) const;
    /// Set the thread name to associate with this id
    void setThreadName(const std::string &name, const std::thread::id &id = std::this_thread::get_id());
    /// Clear the name associated with this id
    void clearThreadName(const std::thread::id &id = std::this_thread::get_id());

private:
    void writeHeader();
    void writeFooter();

private:
    int profileCount = 0;
    mutable std::recursive_mutex mutex;
    std::ofstream outputStream;
    std::unordered_map<std::thread::id, std::string> thread_names;
};

///
/// @brief The source of the benchmark data
///
class Timer
{
public:
    Timer() = delete;
    ///
    /// @brief Construct a new timer object
    ///
    /// @param name the name of the section
    FORCEINLINE Timer(const std::string name): name(std::move(name))
    {
        startTimepoint = std::chrono::high_resolution_clock::now();
    }
    /// Stop the timer
    FORCEINLINE ~Timer()
    {
        if (!stopped) stop();
    }

    /// Send the result to the Instrumentor
    FORCEINLINE void stop()
    {
        if (stopped) return;

        auto endTimepoint = std::chrono::high_resolution_clock::now();

        Instrumentor::get().writeResult({
            .name = std::move(name),
            .start_timestamp = static_cast<unsigned long long>(
                std::chrono::time_point_cast<std::chrono::nanoseconds>(startTimepoint).time_since_epoch().count()),
            .end_timestamp = static_cast<unsigned long long>(
                std::chrono::time_point_cast<std::chrono::nanoseconds>(endTimepoint).time_since_epoch().count()),
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

    #define PROFILE_SCOPE(name) ::pivot::benchmark::Timer PIVOT_MACRO_EXPENDER(timer, __LINE__)(name)
    #define PROFILE_FUNCTION() PROFILE_SCOPE(::pivot::utils::function_name())

#else

    #define PROFILE_SCOPE(name)
    #define PROFILE_FUNCTION()

#endif
