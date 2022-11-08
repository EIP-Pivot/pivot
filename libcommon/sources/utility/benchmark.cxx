#include "pivot/utility/benchmark.hxx"

#include <iomanip>

namespace pivot::benchmark
{

Instrumentor::Instrumentor() {}

Instrumentor::~Instrumentor()
{
    if (isSessionStarted()) endSession();
}

void Instrumentor::beginSession(const std::string &filename)
{
    outputStream.open(filename);
    writeHeader();
}

void Instrumentor::endSession()
{
    if (!verify(isSessionStarted())) return;
    writeFooter();
    outputStream.close();
    profileCount = 0;
}

bool Instrumentor::isSessionStarted() const { return outputStream.is_open(); }

void Instrumentor::writeResult(const TimerResult &result)
{
    if (!verifyMsg(isSessionStarted(), "No session are started !")) return;

    std::unique_lock lock(mutex);
    if (profileCount++ > 0) outputStream << ",";

    std::string name = result.name;
    std::replace(name.begin(), name.end(), '"', '\'');

    // No need to do fancy json serialization, we want to be as fast as possible
    outputStream << "{\"cat\":\"function\",\"dur\":" << (result.end_timestamp - result.start_timestamp)
                 << ",\"name\":\"" << name << "\",\"ph\":\"X\",\"pid\":0,\"tid\": \"" << getThreadName(result.threadId)
                 << "\",\"ts\":" << result.start_timestamp << "}";

    outputStream.flush();
}

std::string Instrumentor::getThreadName(const std::thread::id &id) const
{
    std::unique_lock lock(mutex);
    return (thread_names.contains(id)) ? (thread_names.at(id)) : (std::to_string(std::hash<std::thread::id>()(id)));
}
void Instrumentor::setThreadName(const std::string &name, const std::thread::id &id)
{
    std::unique_lock lock(mutex);
    thread_names[id] = name;
}
void Instrumentor::clearThreadName(const std::thread::id &id)
{
    std::unique_lock lock(mutex);
    thread_names.erase(id);
}

void Instrumentor::writeHeader()
{
    outputStream << "{\"otherData\": {},\"traceEvents\":[";
    outputStream.flush();
}

void Instrumentor::writeFooter()
{
    outputStream << "]}";
    outputStream.flush();
}

}    // namespace pivot::benchmark
