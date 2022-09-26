#include "pivot/utility/benchmark.hxx"

#include <iomanip>

namespace pivot::benchmark
{

Instrumentor::Instrumentor() {}

Instrumentor::~Instrumentor() {}

void Instrumentor::beginSession(const std::string &filename)
{
    outputStream.open(filename);
    writeHeader();
}

void Instrumentor::endSession()
{
    writeFooter();
    outputStream.close();
    profileCount = 0;
}

void Instrumentor::writeResult(TimerResult result)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (profileCount++ > 0) outputStream << ",";

    std::string name = result.name;
    std::replace(name.begin(), name.end(), '"', '\'');

    outputStream << "{\"cat\":\"function\",\"dur\":" << (result.end_timestamp - result.start_timestamp)
                 << ",\"name\":\"" << name << "\",\"ph\":\"X\",\"pid\":0,\"tid\":" << result.threadId
                 << ",\"ts\":" << result.start_timestamp << "}";

    outputStream.flush();
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
